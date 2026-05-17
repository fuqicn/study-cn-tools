#include "gamewidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <cmath>
#include <cstdlib>

static const double PI = 3.14159265358979323846;

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , m_darkMode(true)
    , m_turretAngle(-PI / 2)
    , m_targetAngle(-PI / 2)
    , m_firing(false)
    , m_spawnTimer(0)
    , m_spawnInterval(2.0)
    , m_wave(1)
    , m_score(0)
    , m_gameOver(false)
    , m_paused(true)
    , m_dt(TICK_MS / 1000.0)
{
    setMouseTracking(true);
    setMinimumSize(600, 400);
    m_timer.setInterval(TICK_MS);
    connect(&m_timer, &QTimer::timeout, this, &GameWidget::tick);
    m_elapsed.start();
    m_timer.start();
}

void GameWidget::setDarkMode(bool dark)
{
    m_darkMode = dark;
    update();
}

void GameWidget::startGame()
{
    m_paused = false;
    m_gameOver = false;
    if (!m_timer.isActive())
        m_timer.start();
}

void GameWidget::pauseGame()
{
    m_paused = true;
}

void GameWidget::resumeGame()
{
    m_paused = false;
}

void GameWidget::resetGame()
{
    m_drones.clear();
    m_turretAngle = -PI / 2;
    m_targetAngle = -PI / 2;
    m_firing = false;
    m_spawnTimer = 0;
    m_spawnInterval = 2.0;
    m_wave = 1;
    m_score = 0;
    m_gameOver = false;
    m_paused = true;
    m_turretPos = QPointF(width() / 2.0, height() / 2.0);
    update();
}

double GameWidget::normalizeAngle(double a) const
{
    while (a > PI) a -= 2 * PI;
    while (a < -PI) a += 2 * PI;
    return a;
}

double GameWidget::angleDiff(double from, double to) const
{
    return normalizeAngle(to - from);
}

bool GameWidget::lineCircleIntersect(const QPointF &p1, const QPointF &p2,
                                      const QPointF &center, double radius) const
{
    QPointF d = p2 - p1;
    QPointF f = p1 - center;
    double a = d.x() * d.x() + d.y() * d.y();
    double b = 2 * (f.x() * d.x() + f.y() * d.y());
    double c = f.x() * f.x() + f.y() * f.y() - radius * radius;
    double disc = b * b - 4 * a * c;
    if (disc < 0) return false;
    disc = std::sqrt(disc);
    double t1 = (-b - disc) / (2 * a);
    double t2 = (-b + disc) / (2 * a);
    // intersection in segment [0..1]?
    if (t1 >= 0 && t1 <= 1) return true;
    if (t2 >= 0 && t2 <= 1) return true;
    return false;
}

void GameWidget::spawnDrone()
{
    Drone d;
    int w = width(), h = height();
    if (w < 10 || h < 10) return;

    int side = std::rand() % 4;
    switch (side) {
    case 0: d.pos = QPointF(std::rand() % w, -15); break;         // top
    case 1: d.pos = QPointF(std::rand() % w, h + 15); break;     // bottom
    case 2: d.pos = QPointF(-15, std::rand() % h); break;        // left
    case 3: d.pos = QPointF(w + 15, std::rand() % h); break;     // right
    }

    // Move toward turret with some randomness
    QPointF dir = m_turretPos - d.pos;
    double len = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
    if (len < 1) len = 1;
    double speed = 40 + std::rand() % 30 + m_wave * 3;
    dir = dir / len;
    // add slight perpendicular wobble
    double wobble = ((std::rand() % 100) - 50) / 100.0;
    QPointF perp(-dir.y(), dir.x());
    d.vel = (dir + perp * wobble * 0.3) * speed;
    d.angle = std::atan2(d.vel.y(), d.vel.x());
    d.hp = 1 + m_wave / 5;
    m_drones.append(d);
}

void GameWidget::tick()
{
    m_dt = m_elapsed.elapsed() / 1000.0;
    m_elapsed.start();
    m_dt = qBound(0.001, m_dt, 0.1);
    m_turretPos = QPointF(width() / 2.0, height() / 2.0);

    if (m_gameOver || m_paused) {
        update();
        return;
    }

    // --- Turret rotation toward mouse ---
    double dx = m_mousePos.x() - m_turretPos.x();
    double dy = m_mousePos.y() - m_turretPos.y();
    if (dx * dx + dy * dy > 1.0)
        m_targetAngle = std::atan2(dy, dx);

    double diff = angleDiff(m_turretAngle, m_targetAngle);
    double maxStep = TURRET_ROT_SPEED * m_dt;
    if (std::abs(diff) < maxStep)
        m_turretAngle = m_targetAngle;
    else
        m_turretAngle += (diff > 0 ? maxStep : -maxStep);
    m_turretAngle = normalizeAngle(m_turretAngle);

    // --- Drone spawning ---
    m_spawnTimer += m_dt;
    if (m_spawnTimer >= m_spawnInterval) {
        m_spawnTimer = 0;
        int count = 1 + m_wave / 3;
        for (int i = 0; i < count; ++i) spawnDrone();
        m_wave++;
        m_spawnInterval = std::max(0.6, m_spawnInterval - 0.05);
    }

    // --- Move drones ---
    double deadZone = 28.0;
    for (auto &dr : m_drones) {
        dr.pos += dr.vel * m_dt;
    }

    // --- Laser hit detection ---
    if (m_firing) {
        QPointF laserDir(std::cos(m_turretAngle), std::sin(m_turretAngle));
        QPointF laserEnd = m_turretPos + laserDir * 3000.0;

        QList<int> toRemove;
        for (int i = 0; i < m_drones.size(); ++i) {
            if (lineCircleIntersect(m_turretPos, laserEnd, m_drones[i].pos, 14.0)) {
                m_drones[i].hp -= 1;
                if (m_drones[i].hp <= 0) {
                    toRemove.append(i);
                    m_score += 10;
                }
            }
        }
        // remove in reverse order
        for (int i = toRemove.size() - 1; i >= 0; --i) {
            m_drones.removeAt(toRemove[i]);
        }
    }

    // --- Check if any drone reached turret ---
    for (const auto &dr : m_drones) {
        QPointF diff2 = dr.pos - m_turretPos;
        if (diff2.x() * diff2.x() + diff2.y() * diff2.y() < deadZone * deadZone) {
            m_gameOver = true;
            break;
        }
    }

    // --- Remove off-screen drones that passed through ---
    int w = width(), h = height();
    m_drones.erase(std::remove_if(m_drones.begin(), m_drones.end(),
        [w, h](const Drone &d) {
            return d.pos.x() < -200 || d.pos.x() > w + 200 ||
                   d.pos.y() < -200 || d.pos.y() > h + 200;
        }), m_drones.end());

    update();
}

void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int w = width(), h = height();

    // Background
    QColor bgColor = m_darkMode ? QColor("#0a0a0a") : QColor("#e8e8e8");
    QColor gridColor = m_darkMode ? QColor("#181818") : QColor("#d0d0d0");
    QColor textColor = m_darkMode ? QColor("#ffffff") : QColor("#1a1a1a");
    QColor dimText = m_darkMode ? QColor("#888888") : QColor("#666666");

    p.fillRect(rect(), bgColor);

    // Grid
    p.setPen(QPen(gridColor, 1));
    int gs = 50;
    for (int x = gs; x < w; x += gs) p.drawLine(x, 0, x, h);
    for (int y = gs; y < h; y += gs) p.drawLine(0, y, w, y);

    // === Laser beam ===
    if (m_firing && !m_gameOver) {
        QPointF laserDir(std::cos(m_turretAngle), std::sin(m_turretAngle));
        QPointF laserEnd = m_turretPos + laserDir * 3000.0;

        // Glow
        p.setPen(QPen(QColor(255, 50, 50, 60), 12, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(m_turretPos, laserEnd);
        // Core
        p.setPen(QPen(QColor(255, 80, 80, 200), 3, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(m_turretPos, laserEnd);
        // Bright center
        p.setPen(QPen(QColor(255, 200, 200, 220), 1, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(m_turretPos, laserEnd);
    }

    // === Drones ===
    for (const auto &dr : m_drones) {
        p.save();
        p.translate(dr.pos);
        p.rotate(dr.angle * 180.0 / PI);

        // Body
        QColor droneBody = m_darkMode ? QColor("#556677") : QColor("#889aaa");
        p.setPen(QPen(m_darkMode ? QColor("#7799aa") : QColor("#667788"), 1.5));
        p.setBrush(droneBody);

        // Simple drone shape: triangle-ish
        QPolygonF droneShape;
        droneShape << QPointF(12, 0)
                   << QPointF(-8, -9)
                   << QPointF(-4, 0)
                   << QPointF(-8, 9);
        p.drawPolygon(droneShape);

        // LED indicator
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#ff3333"));
        p.drawEllipse(QPointF(4, 0), 2, 2);

        p.restore();
    }

    // === Turret base (chassis) ===
    double chassisW = 48, chassisH = 30;
    QColor chassisColor = m_darkMode ? QColor("#2a3a2a") : QColor("#5a7a5a");
    QColor chassisBorder = m_darkMode ? QColor("#3a5a3a") : QColor("#4a6a4a");

    p.setPen(QPen(chassisBorder, 2));
    p.setBrush(chassisColor);

    // Treads
    QRectF leftTread(m_turretPos.x() - chassisW/2, m_turretPos.y() - chassisH/2 - 3,
                     chassisW, 7);
    QRectF rightTread(m_turretPos.x() - chassisW/2, m_turretPos.y() + chassisH/2 - 4,
                      chassisW, 7);
    p.drawRoundedRect(leftTread, 2, 2);
    p.drawRoundedRect(rightTread, 2, 2);

    // Hull
    QRectF hull(m_turretPos.x() - chassisW/2 + 4, m_turretPos.y() - chassisH/2 + 2,
                chassisW - 8, chassisH - 4);
    p.setBrush(m_darkMode ? QColor("#3a4a3a") : QColor("#6a8a6a"));
    p.drawRoundedRect(hull, 3, 3);

    // === Turret barrel ===
    p.save();
    p.translate(m_turretPos);
    p.rotate(m_turretAngle * 180.0 / PI);

    // Barrel housing (turret dome)
    QColor turretDomeColor = m_darkMode ? QColor("#4a5a4a") : QColor("#7a9a7a");
    p.setPen(QPen(m_darkMode ? QColor("#5a7a5a") : QColor("#6a8a6a"), 1.5));
    p.setBrush(turretDomeColor);
    p.drawEllipse(QPointF(0, 0), 13, 13);

    // Barrel
    QColor barrelColor = m_darkMode ? QColor("#5a6a5a") : QColor("#8aaa8a");
    p.setBrush(barrelColor);
    p.setPen(QPen(m_darkMode ? QColor("#6a8a6a") : QColor("#7a9a7a"), 1));
    p.drawRoundedRect(QRectF(8, -3, 24, 6), 2, 2);

    // Barrel tip (muzzle)
    if (m_firing && !m_gameOver) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 100, 100, 180));
        p.drawEllipse(QPointF(34, 0), 5, 5);
    }

    p.restore();

    // === Range circle (subtle) ===
    p.setPen(QPen(m_darkMode ? QColor("#222222") : QColor("#cccccc"), 1, Qt::DotLine));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(m_turretPos, 100, 100);

    // === HUD ===
    QFont hudFont("Microsoft YaHei");
    hudFont.setPixelSize(14);
    p.setFont(hudFont);

    // Score
    p.setPen(textColor);
    p.drawText(16, 28, QString::fromUtf8("得分: %1").arg(m_score));
    p.drawText(16, 50, QString::fromUtf8("波次: %1").arg(m_wave));

    // Drone count
    p.setPen(dimText);
    p.drawText(16, 72, QString::fromUtf8("敌机: %1").arg(m_drones.size()));

    // === Game Over overlay ===
    if (m_gameOver) {
        p.fillRect(rect(), QColor(0, 0, 0, 120));

        hudFont.setPixelSize(36);
        p.setFont(hudFont);
        p.setPen(QColor("#ff4444"));
        QString goText = QString::fromUtf8("游戏结束");
        QFontMetrics fm(hudFont);
        int tw = fm.horizontalAdvance(goText);
        p.drawText(w/2 - tw/2, h/2 - 30, goText);

        hudFont.setPixelSize(16);
        p.setFont(hudFont);
        p.setPen(QColor("#cccccc"));
        QString scoreText = QString::fromUtf8("最终得分: %1").arg(m_score);
        tw = fm.horizontalAdvance(scoreText);
        // re-measure with new font
        QFontMetrics fm2(hudFont);
        tw = fm2.horizontalAdvance(scoreText);
        p.drawText(w/2 - tw/2, h/2 + 10, scoreText);

        hudFont.setPixelSize(13);
        p.setFont(hudFont);
        p.setPen(QColor("#888888"));
        QString hint = QString::fromUtf8("点击重新开始");
        QFontMetrics fm3(hudFont);
        tw = fm3.horizontalAdvance(hint);
        p.drawText(w/2 - tw/2, h/2 + 45, hint);
    }

    // Controls hint (top right)
    if (!m_gameOver) {
        hudFont.setPixelSize(11);
        p.setFont(hudFont);
        p.setPen(m_darkMode ? QColor("#444444") : QColor("#aaaaaa"));
        p.drawText(w - 180, 20, QString::fromUtf8("鼠标瞄准 · 按住射击"));
    }

    // Paused overlay
    if (m_paused && !m_gameOver) {
        p.fillRect(rect(), QColor(0, 0, 0, 100));
        hudFont.setPixelSize(32);
        p.setFont(hudFont);
        p.setPen(QColor("#ffd700"));
        QString pauseText = QString::fromUtf8("已暂停");
        QFontMetrics fm(hudFont);
        int tw = fm.horizontalAdvance(pauseText);
        p.drawText(w/2 - tw/2, h/2, pauseText);
    }
}

void GameWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePos = event->pos();
}

void GameWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_gameOver) {
            resetGame();
            return;
        }
        m_firing = true;
    }
}

void GameWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_firing = false;
}

void GameWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_turretPos = QPointF(width() / 2.0, height() / 2.0);
    m_mousePos = m_turretPos;
}
