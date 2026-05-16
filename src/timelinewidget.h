#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QWidget>
#include <QList>

class TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineWidget(QWidget *parent = nullptr);

    struct Item {
        QString period;
        QString yearText;
        QString title;
        QString desc;
    };

    void setItems(const QList<Item> &items);
    void setDarkMode(bool dark);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QList<Item> m_items;
    double m_centerOffset;     // which node index is at screen center
    double m_pixelsPerNode;    // pixel distance between adjacent nodes
    bool m_dragging;
    QPoint m_lastPos;
    bool m_darkMode;

    double nodeToX(double nodeIndex) const;
    double xToNode(double x) const;
};

#endif // TIMELINEWIDGET_H
