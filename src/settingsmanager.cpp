#include "settingsmanager.h"
#include <QApplication>
#include <QStringList>

QString SettingsManager::defaultSettingsPath()
{
    QString appDir = QApplication::applicationDirPath();
    return appDir + "/settings.ini";
}

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(defaultSettingsPath(), QSettings::IniFormat, this))
{
}

QString SettingsManager::theme() const
{
    return m_settings->value("theme", "system").toString();
}

void SettingsManager::setTheme(const QString &theme)
{
    m_settings->setValue("theme", theme);
}

int SettingsManager::fontSize() const
{
    return m_settings->value("fontSize", 14).toInt();
}

void SettingsManager::setFontSize(int size)
{
    m_settings->setValue("fontSize", size);
}

bool SettingsManager::acrylicMode() const
{
    return m_settings->value("acrylicMode", false).toBool();
}

void SettingsManager::setAcrylicMode(bool enabled)
{
    m_settings->setValue("acrylicMode", enabled);
}

bool SettingsManager::sidebarAutoCollapse() const
{
    return m_settings->value("sidebarAutoCollapse", true).toBool();
}

void SettingsManager::setSidebarAutoCollapse(bool enabled)
{
    m_settings->setValue("sidebarAutoCollapse", enabled);
}



QString SettingsManager::customCssPath() const
{
    return m_settings->value("customCssPath", "").toString();
}

void SettingsManager::setCustomCssPath(const QString &path)
{
    m_settings->setValue("customCssPath", path);
}

QString SettingsManager::aiProvider() const
{
    return m_settings->value("aiProvider", "ollama").toString();
}

void SettingsManager::setAiProvider(const QString &provider)
{
    m_settings->setValue("aiProvider", provider);
}

QString SettingsManager::ollamaModel() const
{
    return m_settings->value("ollamaModel", "").toString();
}

void SettingsManager::setOllamaModel(const QString &model)
{
    m_settings->setValue("ollamaModel", model);
}

QString SettingsManager::deepseekModel() const
{
    return m_settings->value("deepseekModel", "").toString();
}

void SettingsManager::setDeepseekModel(const QString &model)
{
    m_settings->setValue("deepseekModel", model);
}

QString SettingsManager::systemPrompt() const
{
    return m_settings->value("systemPrompt", defaultSystemPrompt()).toString();
}

void SettingsManager::setSystemPrompt(const QString &prompt)
{
    m_settings->setValue("systemPrompt", prompt);
}

QString SettingsManager::ollamaUrl() const
{
    return m_settings->value("ollamaUrl", "http://localhost:11434").toString();
}

void SettingsManager::setOllamaUrl(const QString &url)
{
    m_settings->setValue("ollamaUrl", url);
}

QString SettingsManager::deepseekApiKey() const
{
    return m_settings->value("deepseekApiKey", "").toString();
}

void SettingsManager::setDeepseekApiKey(const QString &key)
{
    m_settings->setValue("deepseekApiKey", key);
}

QString SettingsManager::deepseekApiUrl() const
{
    return m_settings->value("deepseekApiUrl", "https://api.deepseek.com/v1").toString();
}

void SettingsManager::setDeepseekApiUrl(const QString &url)
{
    m_settings->setValue("deepseekApiUrl", url);
}

bool SettingsManager::useCustomCss() const
{
    return m_settings->value("useCustomCss", false).toBool();
}

void SettingsManager::setUseCustomCss(bool use)
{
    m_settings->setValue("useCustomCss", use);
}

bool SettingsManager::firstRunCompleted() const
{
    return m_settings->value("firstRunCompleted", false).toBool();
}

void SettingsManager::setFirstRunCompleted(bool completed)
{
    m_settings->setValue("firstRunCompleted", completed);
}

QStringList SettingsManager::discoveredOllamaModels() const
{
    return m_settings->value("discoveredOllamaModels").toStringList();
}

void SettingsManager::setDiscoveredOllamaModels(const QStringList &models)
{
    m_settings->setValue("discoveredOllamaModels", models);
}

QStringList SettingsManager::discoveredDeepseekModels() const
{
    return m_settings->value("discoveredDeepseekModels").toStringList();
}

void SettingsManager::setDiscoveredDeepseekModels(const QStringList &models)
{
    m_settings->setValue("discoveredDeepseekModels", models);
}

void SettingsManager::sync()
{
    m_settings->sync();
}

QString SettingsManager::defaultSystemPrompt()
{
    return R"(你是一位专业的国防安全科普助手。你的职责是：
1. 准确回答关于国防历史、军事科技、战略思想等方面的问题
2. 用通俗易懂的语言解释复杂的军事概念
3. 传播爱国主义精神和国防意识
4. 强调中国国防的防御性质和和平发展理念
5. 避免涉及敏感军事机密信息

请基于公开资料和权威信息回答问题，保持客观、准确、积极的立场。)";
}
