#include "ollamamanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>

static QString normalizeUrl(const QString &url)
{
    QString r = url.trimmed();
    while (r.endsWith('/')) r.chop(1);
    return r;
}

OllamaManager::OllamaManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_available(false)
    , m_currentReply(nullptr)
{
}

/* ------------------------------------------------------------------ */
/*  Status check                                                       */
/* ------------------------------------------------------------------ */
void OllamaManager::checkOllamaStatus(const QString &url)
{
    QUrl apiUrl(normalizeUrl(url) + "/api/tags");
    QNetworkRequest request(apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &OllamaManager::onStatusReplyFinished);
}

void OllamaManager::onStatusReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QStringList modelList;
        for (const QJsonValue &v : doc.object().value("models").toArray()) {
            QString name = v.toObject().value("name").toString();
            if (!name.isEmpty()) modelList.append(name);
        }
        if (!modelList.isEmpty()) {
            m_available = true;
            m_currentModel = modelList.first();
            emit statusChanged(true, "已连接，" + QString::number(modelList.size()) + " 个模型");
            emit modelsFetched(modelList);
        } else {
            m_available = false;
            emit statusChanged(false, "已响应，但未找到模型");
        }
    } else {
        m_available = false;
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        emit statusChanged(false, status == 404 ? "地址无效(404)" : "未启动或无法连接");
    }
    reply->deleteLater();
}

/* ------------------------------------------------------------------ */
/*  Send chat message (streaming)                                      */
/* ------------------------------------------------------------------ */
void OllamaManager::sendMessage(const QString &message, const QString &model,
                                 const QString &systemPrompt, const QString &url)
{
    QString actualModel = model.isEmpty() ? m_currentModel : model;
    if (actualModel.isEmpty()) {
        emit errorOccurred("模型名称为空，请先检查 Ollama 连接");
        return;
    }

    QUrl apiUrl(normalizeUrl(url) + "/api/generate");
    QNetworkRequest request(apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Expect", "");          // prevent 100-continue issues

    QJsonObject json;
    json["model"]  = actualModel;
    json["prompt"] = message;
    json["stream"] = true;
    if (!systemPrompt.isEmpty()) json["system"] = systemPrompt;

    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    m_buffer.clear();

    m_currentReply = m_networkManager->post(request, QJsonDocument(json).toJson());
    connect(m_currentReply, &QNetworkReply::finished,    this, &OllamaManager::onChatReplyFinished);
    connect(m_currentReply, &QNetworkReply::readyRead,   this, &OllamaManager::onReadyRead);
}

/*  Core streaming logic — buffer incomplete lines, parse complete ones  */
void OllamaManager::onReadyRead()
{
    if (!m_currentReply) return;
    m_buffer += m_currentReply->readAll();
    processBuffer(false);
}

void OllamaManager::processBuffer(bool isFinal)
{
    while (true) {
        int idx = m_buffer.indexOf('\n');
        if (idx < 0) {
            // No complete line yet — wait for more data
            // Unless this is the final call after reply finished
            if (isFinal && !m_buffer.trimmed().isEmpty()) {
                QByteArray line = m_buffer.trimmed();
                m_buffer.clear();
                QJsonDocument doc = QJsonDocument::fromJson(line);
                if (!doc.isNull()) {
                    QString resp = doc.object().value("response").toString();
                    if (!resp.isEmpty()) emit responseChunk(resp);
                    if (doc.object().value("done").toBool())   emit responseReceived("");
                }
            }
            break;
        }
        QByteArray line = m_buffer.left(idx).trimmed();
        m_buffer.remove(0, idx + 1);
        if (line.isEmpty()) continue;

        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (doc.isNull()) continue;

        QString resp = doc.object().value("response").toString();
        if (!resp.isEmpty()) emit responseChunk(resp);
        if (doc.object().value("done").toBool())   emit responseReceived("");
    }
}

void OllamaManager::onChatReplyFinished()
{
    // Flush any remaining buffered data
    processBuffer(true);

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() != QNetworkReply::NoError) {
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status == 404)
            emit errorOccurred("Ollama 返回 404 — 模型不存在，请确认模型名称正确且已下载");
        else if (status == 400)
            emit errorOccurred("请求参数错误(400)，请确认模型名称正确");
        else
            emit errorOccurred("请求失败: " + reply->errorString());
    }
    reply->deleteLater();
    m_currentReply = nullptr;
}

/* ------------------------------------------------------------------ */
/*  Fetch models                                                       */
/* ------------------------------------------------------------------ */
void OllamaManager::fetchModels(const QString &url)
{
    QUrl apiUrl(normalizeUrl(url) + "/api/tags");
    QNetworkRequest request(apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &OllamaManager::onModelsReplyFinished);
}

void OllamaManager::onModelsReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    if (reply->error() == QNetworkReply::NoError) {
        QStringList modelList;
        for (const QJsonValue &v : QJsonDocument::fromJson(reply->readAll()).object().value("models").toArray()) {
            QString name = v.toObject().value("name").toString();
            if (!name.isEmpty()) modelList.append(name);
        }
        emit modelsFetched(modelList);
    }
    reply->deleteLater();
}
