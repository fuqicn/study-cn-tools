#ifndef TUTORIALDIALOG_H
#define TUTORIALDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QMouseEvent>

class MainWindow;

class TutorialOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit TutorialOverlay(QWidget *target, const QString &title, const QString &desc,
                             const QString &btnText, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void nextClicked();

private:
    QWidget *m_target;
    QString m_title;
    QString m_desc;
    QString m_btnText;
    QRect m_targetRect;
    QRect m_tooltipRect;
    QRect m_btnRect;
};

class TutorialDialog : public QObject
{
    Q_OBJECT
public:
    explicit TutorialDialog(MainWindow *mainWindow, QObject *parent = nullptr);

    void start();

signals:
    void tutorialFinished();

private slots:
    void goToNextStep();

private:
    void showCurrentStep();
    void createOverlay();
    void removeOverlay();

    MainWindow *m_mainWindow;
    TutorialOverlay *m_overlay;

    int m_currentStep;

    struct TutorialStep {
        QString targetObjectName;  // widget to highlight
        QString title;
        QString description;
        QString buttonText;  // text on the tooltip button
    };
    QList<TutorialStep> m_steps;
};

#endif // TUTORIALDIALOG_H
