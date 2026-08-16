/*
 * SPDX-License-Identifier: MIT
 *
 * 国防安全科普教育软件 (DefenseEdu)
 * Copyright (c) 2026 傅琪 (Fu Qi)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
    QString m_currentProvider;
    quint64 m_requestSerial;
    quint64 m_currentSerial;
    bool m_streamFinished;

    void processBuffer(bool isFinal);
    void maybeEmitResponseReceived();
};

#endif // AISERVICEMANAGER_H
