#ifndef AISERVICEMANAGER_H
#define AISERVICEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QByteArray>

class AiServiceManager : public QObject
{
    Q_OBJECT
public:
    explicit AiServiceManager(QObject *parent = nullptr);

    // provider: "ollama" 或 "deepseek"
    void checkService(const QString &provider, const QString &url, const QString &apiKey = "");
    void sendMessage(const QString &message, const QString &model, const QString &systemPrompt,
                    const QString &provider, const QString &url, const QString &apiKey = "");
    void fetchModels(const QString &provider, const QString &url, const QString &apiKey = "");

    bool isAvailable() const { return m_available; }
    QString currentModel() const { return m_currentModel; }

signals:
    void statusChanged(bool available, const QString &message);
    void responseReceived(const QString &response);
    void responseChunk(const QString &chunk);
    void modelsFetched(const QStringList &models);
    void errorOccurred(const QString &error);

private slots:
    void onStatusReplyFinished();
    void onChatReplyFinished();
    void onModelsReplyFinished();
    void onReadyRead();

private:
    QNetworkAccessManager *m_networkManager;
    bool m_available;
    QString m_currentModel;
    QNetworkReply *m_currentReply;
    QByteArray m_buffer;
    QString m_currentProvider;  // "ollama" 或 "deepseek"
    quint64 m_requestSerial;
    quint64 m_currentSerial;

    void processBuffer(bool isFinal);
};

#endif // AISERVICEMANAGER_H
