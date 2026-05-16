#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
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
