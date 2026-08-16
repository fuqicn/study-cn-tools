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

#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>

class GameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GameWidget(QWidget *parent = nullptr);
    void setDarkMode(bool dark);
    void resetGame();
    void startGame();
    void pauseGame();
    void resumeGame();
    bool isPaused() const { return m_paused; }
    bool isGameOver() const { return m_gameOver; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void tick();

private:
    struct Drone {
        QPointF pos;
        QPointF vel;
        double angle;   // facing angle
        double hp;
    };

    bool m_darkMode;

    // turret
    QPointF m_turretPos;
    double  m_turretAngle;      // current angle (radians)
    double  m_targetAngle;      // angle toward mouse
    static constexpr double TURRET_ROT_SPEED = 3.5; // rad/s

    // laser
    bool    m_firing;
    QPointF m_mousePos;

    // drones
    QList<Drone> m_drones;
    double  m_spawnTimer;
    double  m_spawnInterval;
    int     m_wave;
    int     m_score;
    bool    m_gameOver;
    bool    m_paused;

    // timer
    QTimer m_timer;
    QElapsedTimer m_elapsed;
    static constexpr int TICK_MS = 16;  // ~60fps
    double m_dt;

    // helpers
    void spawnDrone();
    double normalizeAngle(double a) const;
    double angleDiff(double from, double to) const;
    bool lineCircleIntersect(const QPointF &p1, const QPointF &p2,
                             const QPointF &center, double radius) const;
};

#endif // GAMEWIDGET_H
