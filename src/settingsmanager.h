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

    // 默认提示词
    static QString defaultSystemPrompt();

    // 强制同步到磁盘
    void sync();

private:
    QSettings *m_settings;
};

#endif // SETTINGSMANAGER_H
