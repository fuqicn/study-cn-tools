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
    bool eventFilter(QObject *watched, QEvent *event) override;

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
