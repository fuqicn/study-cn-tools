#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QPushButton>
#include <QFrame>
#include <QPropertyAnimation>
#include <QList>
#include <QVariantMap>
#include <QStackedWidget>
#include "aiservicemanager.h"
#include "settingsmanager.h"
#include "timelinewidget.h"
#include "gamewidget.h"

struct EquipmentInfo {
    QString id, name, category, shortDesc, fullDesc, specs;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /* tutorial */
    void startTutorial();
    QList<QPushButton*> getNavButtons() const { return m_navButtons; }
    void openSidebarForTutorial();
    void showPageForTutorial(int index);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onAiServiceStatusChanged(bool available, const QString &message);
    void onResponseChunk(const QString &chunk);
    void onResponseReceived(const QString &);
    void onModelsFetched(const QStringList &models);
    void onErrorOccurred(const QString &error);
    void onQuizResponseChunk(const QString &chunk);
    void onQuizResponseReceived(const QString &);
    void onQuizError(const QString &error);

private:
    void setupUi();
    void initEquipmentData();
    void applyTheme();

    /* sidebar */
    void toggleSidebar();
    void openSidebar();
    void closeSidebar();

    /* pages */
    QWidget* createHomePage();
    QWidget* createTimelinePage();
    QWidget* createChatPage();
    QWidget* createQuizPage();
    QWidget* createGamePage();
    QWidget* createSettingsPage();
    QWidget* createKnowledgeQAPage();
    QWidget* createKnowledgeQuizPage();

    /* detail dialog */
    void showEquipmentDetail(const EquipmentInfo &eq);

    /* navigation */
    void showPage(int index);
    void setActiveNav(int index);

    /* chat */
    void checkAiService();
    void sendChatMessage();

    /* quiz */
    void generateQuiz();
    void showCurrentQuizQuestion();
    void selectAnswer(int idx);
    void nextQuestion();
    void finishQuiz();

    /* knowledge quiz */
    void startKnowledgeQuiz();
    void showCurrentKQuizQuestion();
    void selectKAnswer(int idx);
    void nextKQuestion();
    void finishKQuiz();

    /* settings */
    void saveSettings();
    void resetSettings();
    void loadSettingsToUi();
    void showChangelog();

    /* startup model detection */
    void startupModelDetection();
    QStringList parseDeepseekModelsFromData(const QJsonArray &data);
    QStringList parseDeepseekModelsFromModels(const QJsonArray &models);

    static QString escapeHtml(const QString &text);
    QString renderMarkdown(const QString &md) const;
    QString processInline(const QString &text) const;

    bool m_darkMode;
    QString m_themeMode;  // "system", "dark", "light"
    bool m_acrylicMode;   // Win11 acrylic-style visual effects
    bool m_sidebarAutoCollapse; // auto-collapse sidebar after navigation

    /* animations */
    QList<QPropertyAnimation*> m_cardAnims;

    /* top bar */
    QPushButton *m_menuBtn;
    QLabel      *m_topTitle;

    /* sidebar drawer — absolute positioned */
    QFrame      *m_drawer;
    QPushButton *m_drawerCloseBtn;
    QList<QPushButton*> m_navButtons;
    bool         m_drawerOpen;

    /* content */
    QStackedWidget *m_stack;

    /* home */
    QList<EquipmentInfo> m_equipmentList;

    /* timeline */
    TimelineWidget *m_timelineWidget;

    /* game */
    GameWidget *m_gameWidget;

    /* chat */
    QLabel      *m_statusDot;
    QLabel      *m_statusLabel;
    QTextEdit   *m_chatDisplay;
    QTextEdit   *m_chatInput;

    /* quiz */
    QComboBox   *m_quizDifficulty;
    QLabel      *m_quizProgress;
    QTextEdit   *m_quizQuestion;
    QList<QPushButton*> m_quizButtons;
    QLabel      *m_quizFeedback;
    QLabel      *m_quizScoreLabel;
    QPushButton *m_quizNextBtn;
    int          m_quizCurrent, m_quizCorrect, m_quizTotal, m_quizSelectedAnswer;
    QList<QVariantMap> m_quizQuestions;
    QString      m_quizRawResponse;
    bool         m_quizReceiving;

    /* knowledge quiz */
    QLabel      *m_kQuizProgress;
    QTextEdit   *m_kQuizQuestion;
    QList<QPushButton*> m_kQuizButtons;
    QLabel      *m_kQuizFeedback;
    QLabel      *m_kQuizScoreLabel;
    QPushButton *m_kQuizNextBtn;
    int          m_kQuizCurrent, m_kQuizCorrect, m_kQuizTotal, m_kQuizSelectedAnswer;
    QList<QVariantMap> m_kQuizQuestions;

    /* settings */
    QLineEdit   *m_urlInput;
    QComboBox   *m_modelCombo;
    QComboBox   *m_modelSelectCombo;  // for model selection in settings (dropdown)
    QTextEdit   *m_promptInput;
    QPushButton *m_darkBtn;
    QPushButton *m_lightBtn;
    QPushButton *m_autoBtn;

    /* AI provider settings */
    QComboBox   *m_aiProviderCombo;
    QStackedWidget *m_aiSettingsStack;
    QLineEdit   *m_deepseekApiKeyInput;
    QLineEdit   *m_deepseekApiUrlInput;

    /* managers */
    AiServiceManager    *m_aiServiceManager;
    QNetworkAccessManager *m_startupNetManager;  // for silent startup detection
    SettingsManager *m_settingsManager;

    /* 聊天状态 */
    QString m_chatPrefix;
    QString m_currentResponse;
    bool    m_isReceiving;
};

#endif // MAINWINDOW_H
