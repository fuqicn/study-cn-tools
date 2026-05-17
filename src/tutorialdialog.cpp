#include "tutorialdialog.h"
#include "mainwindow.h"
#include <QApplication>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QPainterPath>

TutorialOverlay::TutorialOverlay(QWidget *target, const QString &title, const QString &desc,
                                 const QString &btnText, QWidget *parent)
    : QWidget(parent)
    , m_target(target)
    , m_title(title)
    , m_desc(desc)
    , m_btnText(btnText)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::FramelessWindowHint);
}

void TutorialOverlay::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Calculate target rect in overlay (parent-relative) coordinates
    if (m_target) {
        QWidget *p = parentWidget();
        m_targetRect = QRect(m_target->mapTo(p, QPoint(0, 0)), m_target->size());
    } else {
        m_targetRect = QRect(width() / 2 - 50, height() / 2 - 50, 100, 100);
    }

    int padding = 8;
    QRect clearRect = m_targetRect.adjusted(-padding, -padding, padding, padding);

    // Draw dark semi-transparent background with a hole using QPainterPath
    QPainterPath path;
    path.addRect(rect());
    path.addRoundedRect(clearRect, 8, 8);

    painter.fillPath(path, QColor(0, 0, 0, 180));

    // Draw border around target
    QPen pen(QColor(196, 30, 58), 3);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(clearRect, 8, 8);

    // Calculate tooltip position
    int tooltipWidth = 320;
    int tooltipHeight = 160;
    int tooltipX = clearRect.right() + 20;
    int tooltipY = clearRect.top() - 20;

    // If tooltip would go off right edge, show on left
    if (tooltipX + tooltipWidth > width() - 20) {
        tooltipX = clearRect.left() - tooltipWidth - 20;
    }
    // If tooltip would go off top, show below
    if (tooltipY < 20) {
        tooltipY = clearRect.bottom() + 20;
    }
    // Clamp to screen bounds
    tooltipX = qBound(20, tooltipX, width() - tooltipWidth - 20);
    tooltipY = qBound(20, tooltipY, height() - tooltipHeight - 20);

    m_tooltipRect = QRect(tooltipX, tooltipY, tooltipWidth, tooltipHeight);

    // Draw tooltip background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255));
    painter.drawRoundedRect(m_tooltipRect, 12, 12);

    // Draw title
    QFont titleFont = painter.font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(QColor(196, 30, 58));

    QRect titleRect = m_tooltipRect.adjusted(16, 16, -16, 0);
    titleRect.setHeight(30);
    painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, m_title);

    // Draw description
    QFont descFont = painter.font();
    descFont.setPointSize(11);
    descFont.setBold(false);
    painter.setFont(descFont);
    painter.setPen(QColor(80, 80, 80));

    QRect descRect = m_tooltipRect.adjusted(16, 46, -16, -50);
    painter.drawText(descRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, m_desc);

    // Draw button
    int btnWidth = 120;
    int btnHeight = 36;
    int btnX = m_tooltipRect.right() - btnWidth - 16;
    int btnY = m_tooltipRect.bottom() - btnHeight - 12;
    m_btnRect = QRect(btnX, btnY, btnWidth, btnHeight);

    painter.setBrush(QColor(196, 30, 58));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(m_btnRect, 6, 6);

    QFont btnFont = painter.font();
    btnFont.setPointSize(12);
    btnFont.setBold(true);
    painter.setFont(btnFont);
    painter.setPen(Qt::white);
    painter.drawText(m_btnRect, Qt::AlignCenter, m_btnText);
}

void TutorialOverlay::mousePressEvent(QMouseEvent *event)
{
    if (m_btnRect.isValid() && m_btnRect.contains(event->pos())) {
        emit nextClicked();
        return;
    }
    // 点击高亮区域或外部区域时不做任何事（以避免穿透干扰下层控件，也不需要跳过步骤）
    event->ignore();
}


TutorialDialog::TutorialDialog(MainWindow *mainWindow, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_overlay(nullptr)
    , m_currentStep(0)
{
    m_steps = {
        {"hamburgerBtn", "打开导航菜单", "点击左上角的菜单按钮，可以打开侧边导航栏，快速切换到各个功能页面。", "点击菜单按钮"},
        {"navBtn_0", "首页 - 国防装备展示", "这里是软件的首页，展示了9种中国自主研发的主力国防装备。点击装备卡片可以查看详细介绍和参数。", "我知道了"},
        {"", "时间轴 - 发展历程", "通过侧边栏导航切换到时间轴页面，查看中国国防建设的发展历程。拖动时间轴浏览从南昌起义到未来愿景的完整历程。", "点击侧边栏"},
        {"", "AI 智能问答", "切换到AI问答页面，AI可以回答您关于国防知识的任何问题。在输入框中输入问题，点击发送或按Enter提交。", "去看看"},
        {"", "AI 智能答题", "AI会自动生成国防知识测试题目，选择答题数量后点击生成题目，选择答案后即时判分。", "去试试"},
        {"", "知识问答", "知识问答页面预制了12道国防知识问答题，点击问题卡片可以查看详细解答。", "去看看"},
        {"", "知识答题", "知识答题页面预制了20道国防知识选择题，测试您的国防知识水平，选择答案后即时反馈。", "去答题"},
        {"", "激光防御模拟", "2D炮塔防御小游戏：炮管会朝向鼠标位置，按住鼠标左键持续发射激光，消灭从边缘逼近的无人机。", "玩游戏"},
        {"", "软件设置", "在设置页面，您可以自定义主题、字体大小、AI服务商等参数，还可以查看更新日志。", "完成教程"}
    };
}

void TutorialDialog::start()
{
    m_currentStep = 0;
    showCurrentStep();
}

void TutorialDialog::createOverlay()
{
    removeOverlay();

    QWidget *target = nullptr;
    const TutorialStep &step = m_steps[m_currentStep];

    if (!step.targetObjectName.isEmpty()) {
        if (step.targetObjectName.startsWith("navBtn_")) {
            // Special handling for nav buttons
            int navIdx = step.targetObjectName.mid(7).toInt();
            if (navIdx >= 0 && navIdx < m_mainWindow->getNavButtons().size()) {
                target = m_mainWindow->getNavButtons()[navIdx];
            }
        } else {
            target = m_mainWindow->findChild<QWidget*>(step.targetObjectName);
        }
    }

    m_overlay = new TutorialOverlay(target, step.title, step.description, step.buttonText, m_mainWindow);
    m_overlay->setGeometry(m_mainWindow->rect());
    m_overlay->show();
    m_overlay->raise();

    connect(m_overlay, &TutorialOverlay::nextClicked, this, &TutorialDialog::goToNextStep);
}

void TutorialDialog::removeOverlay()
{
    if (m_overlay) {
        m_overlay->close();
        m_overlay->deleteLater();
        m_overlay = nullptr;
    }
}

void TutorialDialog::showCurrentStep()
{
    if (m_currentStep >= m_steps.size()) {
        removeOverlay();
        emit tutorialFinished();
        return;
    }

    const TutorialStep &step = m_steps[m_currentStep];
    QString targetObj = step.targetObjectName;

    // Handle navigation before showing overlay
    if (targetObj == "hamburgerBtn") {
        // Just show overlay on menu button
    }
    else if (targetObj.startsWith("navBtn_")) {
        m_mainWindow->openSidebarForTutorial();
    }
    else if (m_currentStep > 1) {
        // Navigate to the page, sidebar will auto-close
        int pageIndex = m_currentStep - 1;
        if (pageIndex >= 0 && pageIndex < 8) {
            m_mainWindow->showPageForTutorial(pageIndex);
        }
    }

    // Use QTimer to ensure UI updates before overlay is drawn
    QTimer::singleShot(50, this, [this]() {
        createOverlay();
    });
}

void TutorialDialog::goToNextStep()
{
    m_currentStep++;
    if (m_currentStep >= m_steps.size()) {
        removeOverlay();
        emit tutorialFinished();
    } else {
        showCurrentStep();
    }
}
