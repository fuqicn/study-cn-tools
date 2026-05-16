#ifndef FIRSTRUNDIALOG_H
#define FIRSTRUNDIALOG_H

#include <QDialog>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QTextEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QScrollArea>
#include <QFrame>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QMessageBox>
#include <QButtonGroup>

class FirstRunDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FirstRunDialog(QWidget *parent = nullptr);

    QString theme() const;
    bool licenseAccepted() const;

private:
    void setupUi();
    void goToNextPage();
    void goToPrevPage();
    void updateNavButtons();
    void finishSetup();

    void startModelDetection();
    void onOllamaDetectionFinished();

    QStackedWidget *m_stack;
    QPushButton *m_backBtn;
    QPushButton *m_nextBtn;
    QLabel *m_pageIndicator;

    // Page 0: Welcome
    QWidget *createWelcomePage();

    // Page 1: License
    QWidget *createLicensePage();
    QCheckBox *m_licenseCheck;

    // Page 2: Theme
    QWidget *createThemePage();
    QPushButton *m_themeLightBtn;
    QPushButton *m_themeDarkBtn;
    QPushButton *m_themeSystemBtn;
    QButtonGroup *m_themeGroup;

    // Page 3: Visual Effects
    QWidget *createVisualEffectsPage();
    QCheckBox *m_acrylicCheck;
    QCheckBox *m_sidebarAutoCheck;

    // Page 4: AI Provider Selection & Model Detection
    QWidget *createModelDetectionPage();
    QPushButton *m_ollamaProviderBtn;
    QPushButton *m_deepseekProviderBtn;
    QLabel *m_modelStatusLabel;
    QComboBox *m_modelCombo;
    QLabel *m_modelHintLabel;
    QPushButton *m_skipAiBtn;
    QNetworkAccessManager *m_netManager;
    QStringList m_detectedModels;
    bool m_aiSkipped;

    // Page 5: Features
    QWidget *createFeaturesPage();

    // Page 6: Complete
    QWidget *createCompletePage();
};

#endif // FIRSTRUNDIALOG_H
