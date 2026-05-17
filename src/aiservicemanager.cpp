#include "aiservicemanager.h"
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

AiServiceManager::AiServiceManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_available(false)
    , m_currentReply(nullptr)
    , m_requestSerial(0)
    , m_currentSerial(0)
    , m_streamFinished(false)
{
}

/* ------------------------------------------------------------------ */
/*  Status check                                                       */
/* ------------------------------------------------------------------ */
void AiServiceManager::checkService(const QString &provider, const QString &url, const QString &apiKey)
{
    m_currentProvider = provider;

    if (provider == "ollama") {
        QUrl apiUrl(normalizeUrl(url) + "/api/tags");
        QNetworkRequest request(apiUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply *reply = m_networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, &AiServiceManager::onStatusReplyFinished);
    } else if (provider == "deepseek") {
        // DeepSeek: 检查API是否可用（通过models端点）
        QUrl apiUrl(normalizeUrl(url) + "/models");
        QNetworkRequest request(apiUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
        QNetworkReply *reply = m_networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, &AiServiceManager::onStatusReplyFinished);
    }
}

void AiServiceManager::onStatusReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (m_currentProvider == "ollama") {
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
        } else if (m_currentProvider == "deepseek") {
            // DeepSeek API 返回 {"data": [...]}
            if (doc.object().contains("data") || doc.object().contains("models")) {
                m_available = true;
                QStringList modelList = {"deepseek-chat", "deepseek-coder"};
                m_currentModel = "deepseek-chat";
                emit statusChanged(true, "DeepSeek API 已连接");
                emit modelsFetched(modelList);
            } else {
                m_available = false;
                emit statusChanged(false, "API 响应异常");
            }
        }
    } else {
        m_available = false;
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status == 401) {
            emit statusChanged(false, "API 密钥无效");
        } else if (status == 404) {
            emit statusChanged(false, "地址无效(404)");
        } else {
            emit statusChanged(false, "未启动或无法连接");
        }
    }
    reply->deleteLater();
}

/* ------------------------------------------------------------------ */
/*  Send chat message (streaming)                                      */
/* ------------------------------------------------------------------ */
void AiServiceManager::sendMessage(const QString &message, const QString &model,
                                    const QString &systemPrompt, const QString &provider,
                                    const QString &url, const QString &apiKey)
{
    m_currentProvider = provider;
    QString actualModel = model.isEmpty() ? m_currentModel : model;

    if (actualModel.isEmpty()) {
        emit errorOccurred("模型名称为空，请先检查 AI 服务连接");
        return;
    }

    m_streamFinished = false;

    if (provider == "ollama") {
        QUrl apiUrl(normalizeUrl(url) + "/api/generate");
        QNetworkRequest request(apiUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Expect", "");

        QJsonObject json;
        json["model"]  = actualModel;
        json["prompt"] = message;
        json["stream"] = true;
        if (!systemPrompt.isEmpty()) json["system"] = systemPrompt;

        m_buffer.clear();
        if (m_currentReply) {
            m_currentReply->abort();
            m_currentReply->deleteLater();
            m_currentReply = nullptr;
        }

        m_currentSerial = ++m_requestSerial;
        quint64 mySerial = m_currentSerial;

        m_currentReply = m_networkManager->post(request, QJsonDocument(json).toJson());
        QObject::connect(m_currentReply, &QNetworkReply::finished,  this, &AiServiceManager::onChatReplyFinished);
        QObject::connect(m_currentReply, &QNetworkReply::readyRead, this, &AiServiceManager::onReadyRead);
        QTimer::singleShot(60000, m_currentReply, [this, mySerial]() {
            if (mySerial == m_currentSerial && m_currentReply && m_currentReply->isRunning()) {
                m_currentReply->abort();
                emit errorOccurred("请求超时（60秒）");
            }
        });

    } else if (provider == "deepseek") {
        QUrl apiUrl(normalizeUrl(url) + "/chat/completions");
        QNetworkRequest request(apiUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
        request.setRawHeader("Expect", "");

        QJsonObject json;
        json["model"] = actualModel;
        json["stream"] = true;

        QJsonArray messages;
        if (!systemPrompt.isEmpty()) {
            QJsonObject sysMsg;
            sysMsg["role"] = "system";
            sysMsg["content"] = systemPrompt;
            messages.append(sysMsg);
        }
        QJsonObject userMsg;
        userMsg["role"] = "user";
        userMsg["content"] = message;
        messages.append(userMsg);

        json["messages"] = messages;

        m_buffer.clear();
        if (m_currentReply) {
            m_currentReply->abort();
            m_currentReply->deleteLater();
            m_currentReply = nullptr;
        }

        m_currentSerial = ++m_requestSerial;
        quint64 mySerial = m_currentSerial;

        m_currentReply = m_networkManager->post(request, QJsonDocument(json).toJson());
        QObject::connect(m_currentReply, &QNetworkReply::finished,  this, &AiServiceManager::onChatReplyFinished);
        QObject::connect(m_currentReply, &QNetworkReply::readyRead, this, &AiServiceManager::onReadyRead);
        QTimer::singleShot(60000, m_currentReply, [this, mySerial]() {
            if (mySerial == m_currentSerial && m_currentReply && m_currentReply->isRunning()) {
                m_currentReply->abort();
                emit errorOccurred("请求超时（60秒）");
            }
        });
    }
}

void AiServiceManager::maybeEmitResponseReceived()
{
    if (m_streamFinished) return;
    m_streamFinished = true;
    emit responseReceived("");
}

/*  Core streaming logic — buffer incomplete lines, parse complete ones  */
void AiServiceManager::onReadyRead()
{
    if (!m_currentReply) return;
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply && reply != m_currentReply) return;
    m_buffer += m_currentReply->readAll();
    if (m_buffer.size() > 1024 * 1024) {
        m_currentReply->abort();
        emit errorOccurred("响应数据过大，已中断");
        return;
    }
    processBuffer(false);
}

void AiServiceManager::processBuffer(bool isFinal)
{
    while (true) {
        int idx = m_buffer.indexOf('\n');
        if (idx < 0) {
            if (isFinal && !m_buffer.trimmed().isEmpty()) {
                QByteArray line = m_buffer.trimmed();
                m_buffer.clear();

                if (m_currentProvider == "ollama") {
                    QJsonDocument doc = QJsonDocument::fromJson(line);
                    if (!doc.isNull()) {
                        QString resp = doc.object().value("response").toString();
                        if (!resp.isEmpty()) emit responseChunk(resp);
                        if (doc.object().value("done").toBool()) maybeEmitResponseReceived();
                    }
                } else if (m_currentProvider == "deepseek") {
                    QString lineStr = QString::fromUtf8(line);
                    if (lineStr.startsWith("data: ")) {
                        QString jsonStr = lineStr.mid(6);
                        if (jsonStr != "[DONE]") {
                            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
                            if (!doc.isNull()) {
                                QString content = doc.object().value("choices").toArray()[0].toObject()
                                                      .value("delta").toObject()
                                                      .value("content").toString();
                                if (!content.isEmpty()) emit responseChunk(content);
                                if (doc.object().value("choices").toArray()[0].toObject()
                                        .value("finish_reason").toString() == "stop") {
                                    maybeEmitResponseReceived();
                                }
                            }
                        } else {
                            maybeEmitResponseReceived();
                        }
                    }
                }
            }
            break;
        }
        QByteArray line = m_buffer.left(idx).trimmed();
        m_buffer.remove(0, idx + 1);
        if (line.isEmpty()) continue;

        if (m_currentProvider == "ollama") {
            QJsonDocument doc = QJsonDocument::fromJson(line);
            if (doc.isNull()) continue;

            QString resp = doc.object().value("response").toString();
            if (!resp.isEmpty()) emit responseChunk(resp);
            if (doc.object().value("done").toBool()) maybeEmitResponseReceived();

        } else if (m_currentProvider == "deepseek") {
            QString lineStr = QString::fromUtf8(line);
            if (lineStr.startsWith("data: ")) {
                QString jsonStr = lineStr.mid(6);
                if (jsonStr == "[DONE]") {
                    maybeEmitResponseReceived();
                    break;
                }
                QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
                if (doc.isNull()) continue;

                QJsonArray choices = doc.object().value("choices").toArray();
                if (!choices.isEmpty()) {
                    QString content = choices[0].toObject()
                                          .value("delta").toObject()
                                          .value("content").toString();
                    if (!content.isEmpty()) emit responseChunk(content);

                    QString finishReason = choices[0].toObject().value("finish_reason").toString();
                    if (finishReason == "stop") {
                        maybeEmitResponseReceived();
                        break;
                    }
                }
            }
        }
    }
}

void AiServiceManager::onChatReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    // 忽略被中止或被替换的旧回复（例如切换到答题时中止的聊天请求）
    if (reply != m_currentReply) {
        reply->deleteLater();
        return;
    }
    // 被手动中止的请求，不触发任何信号
    if (reply->error() == QNetworkReply::OperationCanceledError) {
        m_buffer.clear();
        reply->deleteLater();
        m_currentReply = nullptr;
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        processBuffer(true);
        // 兜底：如果流中没有明确的终止标记，在此处确保终止
        maybeEmitResponseReceived();
    } else {
        processBuffer(true);
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status == 401) {
            emit errorOccurred("API 密钥无效，请检查设置");
        } else if (status == 404) {
            if (m_currentProvider == "ollama")
                emit errorOccurred("Ollama 返回 404 — 模型不存在，请确认模型名称正确且已下载");
            else
                emit errorOccurred("DeepSeek API 返回 404 — 地址或模型名称有误");
        } else if (status == 400) {
            emit errorOccurred("请求参数错误(400)，请检查设置");
        } else {
            emit errorOccurred("请求失败: " + reply->errorString());
        }
    }
    reply->deleteLater();
    m_currentReply = nullptr;
}

/* ------------------------------------------------------------------ */
/*  Fetch models                                                       */
/* ------------------------------------------------------------------ */
void AiServiceManager::fetchModels(const QString &provider, const QString &url, const QString &apiKey)
{
    m_currentProvider = provider;

    if (provider == "ollama") {
        QUrl apiUrl(normalizeUrl(url) + "/api/tags");
        QNetworkRequest request(apiUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply *reply = m_networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, &AiServiceManager::onModelsReplyFinished);
    } else if (provider == "deepseek") {
        // DeepSeek 直接返回预定义模型列表
        QStringList models = {"deepseek-chat", "deepseek-coder"};
        emit modelsFetched(models);
    }
}

void AiServiceManager::onModelsReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (m_currentProvider == "ollama" && reply->error() == QNetworkReply::NoError) {
        QStringList modelList;
        for (const QJsonValue &v : QJsonDocument::fromJson(reply->readAll()).object().value("models").toArray()) {
            QString name = v.toObject().value("name").toString();
            if (!name.isEmpty()) modelList.append(name);
        }
        emit modelsFetched(modelList);
    }
    reply->deleteLater();
}
