#ifndef OLLAMAMANAGER_H
#define OLLAMAMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QByteArray>

class OllamaManager : public QObject
{
    Q_OBJECT
public:
    explicit OllamaManager(QObject *parent = nullptr);

    void checkOllamaStatus(const QString &url = "http://localhost:11434");
    void sendMessage(const QString &message, const QString &model, const QString &systemPrompt, const QString &url);
    void fetchModels(const QString &url = "http://localhost:11434");

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

    void processBuffer(bool isFinal);
};

#endif // OLLAMAMANAGER_H
