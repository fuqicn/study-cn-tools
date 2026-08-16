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

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = nullptr);

    // 简易模式设置
    QString theme() const;
    void setTheme(const QString &theme);

    int fontSize() const;
    void setFontSize(int size);

    bool acrylicMode() const;
    void setAcrylicMode(bool enabled);

    bool sidebarAutoCollapse() const;
    void setSidebarAutoCollapse(bool enabled);


    // 专业模式设置
    QString customCssPath() const;
    void setCustomCssPath(const QString &path);

    // AI服务设置
    QString aiProvider() const;  // "ollama" 或 "deepseek"
    void setAiProvider(const QString &provider);

    QString ollamaModel() const;
    void setOllamaModel(const QString &model);

    QString deepseekModel() const;
    void setDeepseekModel(const QString &model);

    QString systemPrompt() const;
    void setSystemPrompt(const QString &prompt);

    QString ollamaUrl() const;
    void setOllamaUrl(const QString &url);

    QString deepseekApiKey() const;
    void setDeepseekApiKey(const QString &key);

    QString deepseekApiUrl() const;
    void setDeepseekApiUrl(const QString &url);

    bool useCustomCss() const;
    void setUseCustomCss(bool use);

    // First run tracking
    bool firstRunCompleted() const;
    void setFirstRunCompleted(bool completed);

    // Discovered models
    QStringList discoveredOllamaModels() const;
    void setDiscoveredOllamaModels(const QStringList &models);
    QStringList discoveredDeepseekModels() const;
    void setDiscoveredDeepseekModels(const QStringList &models);

    // 默认设置文件路径（与 main.cpp 保持一致）
    static QString defaultSettingsPath();

    // 默认提示词
    static QString defaultSystemPrompt();

    // 强制同步到磁盘
    void sync();

private:
    QSettings *m_settings;
};

#endif // SETTINGSMANAGER_H
