#include "timelinewidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFontMetrics>
#include <cmath>
#include <algorithm>

TimelineWidget::TimelineWidget(QWidget *parent)
    : QWidget(parent)
    , m_centerOffset(4.0)      // start with 5th node (index 4) centered
    , m_pixelsPerNode(200)
    , m_dragging(false)
    , m_darkMode(true)
{
    setMinimumHeight(400);
    setCursor(Qt::OpenHandCursor);
}

void TimelineWidget::setItems(const QList<Item> &items)
{
    m_items = items;
    // Center on the middle node
    if (!m_items.isEmpty())
        m_centerOffset = (m_items.size() - 1) / 2.0;
    update();
}

void TimelineWidget::setDarkMode(bool dark)
{
    m_darkMode = dark;
    update();
}

double TimelineWidget::nodeToX(double nodeIndex) const
{
    return (nodeIndex - m_centerOffset) * m_pixelsPerNode + width() / 2.0;
}

double TimelineWidget::xToNode(double x) const
{
    return (x - width() / 2.0) / m_pixelsPerNode + m_centerOffset;
}

void TimelineWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int w = width();
    const int h = height();
    const int lineY = static_cast<int>(h * 0.72);
    const int centerX = w / 2;
    const double maxDist = w / 2.0;
    const int n = m_items.size();

    // Background
    p.fillRect(rect(), m_darkMode ? QColor("#000000") : QColor("#f0f0f0"));

    // === Period zone backgrounds ===
    auto makeZoneColor = [](const char *hex, int alpha) -> QColor {
        QColor c(hex); c.setAlpha(alpha); return c;
    };

    // Find contiguous period ranges by node index
    if (n > 0) {
        QMap<QString, QColor> periodZoneColor = {
            {QString::fromUtf8("过去"), makeZoneColor(m_darkMode ? "#888888" : "#aaaaaa", 22)},
            {QString::fromUtf8("现在"), makeZoneColor("#c41e3a", 22)},
            {QString::fromUtf8("未来"), makeZoneColor("#ffd700", 22)}
        };
        int runStart = 0;
        for (int i = 1; i <= n; ++i) {
            bool endOfRun = (i == n || m_items[i].period != m_items[runStart].period);
            if (endOfRun) {
                QColor zc = periodZoneColor.value(m_items[runStart].period);
                if (zc.isValid()) {
                    double x1 = nodeToX(runStart - 0.4);
                    double x2 = nodeToX(i - 1 + 0.4);
                    p.fillRect(QRectF(x1, lineY - 30, x2 - x1, 60), zc);
                }
                runStart = i;
            }
        }
    }

    // === Number line ===
    QColor lineColor = m_darkMode ? QColor("#555555") : QColor("#bbbbbb");
    p.setPen(QPen(lineColor, 2));
    p.drawLine(0, lineY, w, lineY);

    // Arrow at right end
    QPolygon arrow;
    arrow << QPoint(w - 2, lineY)
          << QPoint(w - 12, lineY - 6)
          << QPoint(w - 12, lineY + 6);
    p.setBrush(lineColor);
    p.setPen(Qt::NoPen);
    p.drawPolygon(arrow);

    // === Tick marks at each node ===
    QColor tickColor = m_darkMode ? QColor("#666666") : QColor("#999999");
    QColor yearLabelColor = m_darkMode ? QColor("#888888") : QColor("#666666");
    QFont yearFont("Microsoft YaHei");
    yearFont.setPixelSize(12);
    p.setFont(yearFont);

    for (int i = 0; i < n; ++i) {
        int x = static_cast<int>(nodeToX(i));
        if (x < -20 || x > w + 20) continue;

        // Tick mark
        p.setPen(QPen(tickColor, 1));
        p.drawLine(x, lineY - 6, x, lineY + 6);

        // Year label below the line
        p.setPen(yearLabelColor);
        QFontMetrics fm(yearFont);
        int tw = fm.horizontalAdvance(m_items[i].yearText);
        p.drawText(x - tw / 2, lineY + 24, m_items[i].yearText);
    }

    // === Center subtle indicator ===
    p.setPen(QPen(QColor(m_darkMode ? "#222222" : "#e0e0e0"), 1, Qt::DashLine));
    p.drawLine(centerX, 10, centerX, lineY - 30);

    // === Sort items by distance from center (farthest drawn first) ===
    QList<int> indices;
    for (int i = 0; i < n; ++i) indices.append(i);
    std::sort(indices.begin(), indices.end(), [this, centerX](int a, int b) {
        return std::abs(nodeToX(a) - centerX) > std::abs(nodeToX(b) - centerX);
    });

    // === Draw cards ===
    for (int idx : indices) {
        const Item &item = m_items[idx];
        int x = static_cast<int>(nodeToX(idx));

        if (x < -250 || x > w + 250) continue;

        // Distance from center (normalized 0..1)
        double dist = std::abs(x - centerX) / maxDist;
        if (dist > 1.0) dist = 1.0;

        // Scale: 1.0 at center, 0.72 at edge
        double scale = 1.0 - 0.28 * dist;

        // Opacity
        int alpha = static_cast<int>(255 - 115 * dist);

        // Period color
        QColor periodColor;
        if (item.period == QString::fromUtf8("过去"))
            periodColor = QColor("#888888");
        else if (item.period == QString::fromUtf8("现在"))
            periodColor = QColor("#c41e3a");
        else
            periodColor = QColor("#ffd700");

        // Card dimensions
        const int baseW = 170;
        const int baseH = 140;
        int cardW = static_cast<int>(baseW * scale);
        int cardH = static_cast<int>(baseH * scale);
        int cardX = x - cardW / 2;
        int cardY = lineY - 28 - cardH;

        // Connector line
        QColor connColor = periodColor;
        connColor.setAlpha(alpha);
        p.setPen(QPen(connColor, qMax(1.0, 2.0 * scale)));
        p.drawLine(x, cardY + cardH, x, lineY);

        // Dot on number line
        p.setPen(Qt::NoPen);
        connColor.setAlpha(alpha);
        p.setBrush(connColor);
        p.drawEllipse(QPointF(x, lineY), 5 * scale, 5 * scale);

        // Card background
        QColor bg = m_darkMode ? QColor("#111111") : QColor("#ffffff");
        bg.setAlpha(alpha);
        p.setPen(Qt::NoPen);
        p.setBrush(bg);
        p.drawRoundedRect(cardX, cardY, cardW, cardH,
                          8 * scale, 8 * scale);

        // Card border
        QColor border = m_darkMode ? QColor("#333333") : QColor("#dddddd");
        border.setAlpha(alpha);
        p.setPen(QPen(border, 1));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(cardX, cardY, cardW, cardH,
                          8 * scale, 8 * scale);

        // Left accent border
        QColor accentBorder = periodColor;
        accentBorder.setAlpha(alpha);
        p.setPen(Qt::NoPen);
        p.setBrush(accentBorder);
        p.drawRoundedRect(QRectF(cardX, cardY + 4 * scale,
                                 3 * scale, cardH - 8 * scale), 1.0, 1.0);

        // Text content
        p.save();
        p.setClipRect(cardX + 1, cardY + 1, cardW - 2, cardH - 2);

        int textX = cardX + static_cast<int>(12 * scale);
        int textW = cardW - static_cast<int>(22 * scale);
        int curY = cardY + static_cast<int>(10 * scale);

        // Period badge
        QFont badgeFont("Microsoft YaHei");
        badgeFont.setPixelSize(std::max(8, static_cast<int>(10 * scale)));
        p.setFont(badgeFont);
        QColor badgeColor = periodColor;
        badgeColor.setAlpha(alpha);
        p.setPen(badgeColor);
        p.drawText(QRect(textX, curY, textW, static_cast<int>(16 * scale)),
                   Qt::AlignLeft | Qt::AlignVCenter, item.period);
        curY += static_cast<int>(18 * scale);

        // Year text
        QFont yrFont("Microsoft YaHei");
        yrFont.setPixelSize(std::max(10, static_cast<int>(16 * scale)));
        yrFont.setBold(true);
        p.setFont(yrFont);
        QColor yrColor = m_darkMode ? QColor("#ffd700") : QColor("#b8860b");
        yrColor.setAlpha(alpha);
        p.setPen(yrColor);
        p.drawText(QRect(textX, curY, textW, static_cast<int>(22 * scale)),
                   Qt::AlignLeft | Qt::AlignVCenter, item.yearText);
        curY += static_cast<int>(24 * scale);

        // Title text
        QFont titleFont("Microsoft YaHei");
        titleFont.setPixelSize(std::max(9, static_cast<int>(13 * scale)));
        titleFont.setBold(true);
        p.setFont(titleFont);
        QColor titleColor = m_darkMode ? QColor("#ffffff") : QColor("#1a1a1a");
        titleColor.setAlpha(alpha);
        p.setPen(titleColor);
        p.drawText(QRect(textX, curY, textW, static_cast<int>(18 * scale)),
                   Qt::AlignLeft | Qt::AlignVCenter, item.title);
        curY += static_cast<int>(20 * scale);

        // Description
        QFont descFont("Microsoft YaHei");
        descFont.setPixelSize(std::max(8, static_cast<int>(11 * scale)));
        p.setFont(descFont);
        QColor descColor = m_darkMode ? QColor("#888888") : QColor("#666666");
        descColor.setAlpha(alpha);
        p.setPen(descColor);
        int descH = cardY + cardH - curY - static_cast<int>(6 * scale);
        if (descH > 0) {
            p.drawText(QRect(textX, curY, textW, descH),
                       Qt::AlignLeft | Qt::TextWordWrap, item.desc);
        }

        p.restore();
    }

    // Hint text
    QFont hintFont("Microsoft YaHei");
    hintFont.setPixelSize(11);
    p.setFont(hintFont);
    p.setPen(m_darkMode ? QColor("#555555") : QColor("#999999"));
    p.drawText(QRect(0, h - 28, w, 22),
               Qt::AlignCenter,
               QString::fromUtf8("← 拖动浏览 · 滚轮缩放 →"));
}

void TimelineWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void TimelineWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        int dx = event->pos().x() - m_lastPos.x();
        m_centerOffset -= dx / m_pixelsPerNode;
        // Clamp to boundaries
        if (!m_items.isEmpty()) {
            m_centerOffset = qBound(-0.5, m_centerOffset, (double)m_items.size() - 0.5);
        }
        m_lastPos = event->pos();
        update();
    }
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        setCursor(Qt::OpenHandCursor);
    }
}

void TimelineWidget::wheelEvent(QWheelEvent *event)
{
    double factor = event->angleDelta().y() > 0 ? 1.1 : 1.0 / 1.1;
    double newPPN = m_pixelsPerNode * factor;
    if (newPPN < 80) newPPN = 80;
    if (newPPN > 600) newPPN = 600;

    // Zoom toward mouse position
    double mouseNode = xToNode(event->position().x());
    m_pixelsPerNode = newPPN;
    m_centerOffset = mouseNode - (event->position().x() - width() / 2.0) / m_pixelsPerNode;
    // Clamp to boundaries
    if (!m_items.isEmpty()) {
        m_centerOffset = qBound(-0.5, m_centerOffset, (double)m_items.size() - 0.5);
    }

    update();
}
