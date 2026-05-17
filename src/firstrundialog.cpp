#include "firstrundialog.h"
#include "settingsmanager.h"
#include <QApplication>
#include <QDir>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

static bool systemDarkMode()
{
#ifdef Q_OS_WIN
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                  QSettings::NativeFormat);
    return reg.value("AppsUseLightTheme", 1).toInt() == 0;
#else
    return false;
#endif
}

FirstRunDialog::FirstRunDialog(QWidget *parent)
    : QDialog(parent)
    , m_stack(new QStackedWidget(this))
    , m_backBtn(new QPushButton("< 上一步"))
    , m_nextBtn(new QPushButton("下一步 >"))
    , m_licenseCheck(nullptr)
    , m_themeLightBtn(nullptr)
    , m_themeDarkBtn(nullptr)
    , m_themeSystemBtn(nullptr)
    , m_themeGroup(nullptr)
    , m_acrylicCheck(nullptr)
    , m_sidebarAutoCheck(nullptr)
    , m_netManager(new QNetworkAccessManager(this))
    , m_aiSkipped(false)
{
    setWindowTitle("国防安全科普教育软件 - 初次设置");
    setMinimumSize(700, 520);
    setMaximumSize(700, 520);
    setModal(true);

    setupUi();
}

void FirstRunDialog::setupUi()
{
    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // Apply a clean modern style
    QString dlgStyle = QString(
        "QDialog { background: #f8f9fa; color: #1a1a1a; }"
        "QPushButton { background: #c41e3a; color: white; border: none; padding: 10px 28px; "
        "border-radius: 6px; font-size: 14px; }"
        "QPushButton:hover { background: #e8294a; }"
        "QPushButton:disabled { background: #cccccc; color: #888888; }"
        "QPushButton#backBtn { background: transparent; color: #666666; border: 1px solid #cccccc; padding: 10px 22px; text-align: center; }"
        "QPushButton#backBtn:hover { background: #e8e8e8; }"
        "QLabel { background: transparent; color: #1a1a1a; }"
        "QTextEdit { background: white; border: 1px solid #dddddd; border-radius: 8px; padding: 12px; color: #333333; }"
        "QCheckBox { color: #333333; }"
        "QSlider::groove:horizontal { height: 6px; background: #e0e0e0; border-radius: 3px; }"
        "QSlider::handle:horizontal { width: 18px; height: 18px; margin:-6px 0; background: #c41e3a; border-radius: 9px; }"
        "QSlider::sub-page:horizontal { background: #c41e3a; border-radius: 3px; }"
        "QFrame#welcomeFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #c41e3a, stop:1 #8b0000); border-radius: 12px; }"
        "QFrame#featureCard { background: white; border: 1px solid #e0e0e0; border-radius: 10px; padding: 12px; }"
        "QLabel#welcomeTitle { font-size: 32px; font-weight: bold; color: white; }"
        "QLabel#welcomeSub { font-size: 16px; color: rgba(255,255,255,0.9); }"
        "QLabel#featureTitle { font-size: 16px; font-weight: bold; color: #c41e3a; }"
        "QLabel#featureDesc { font-size: 13px; color: #555555; }"
        "QLabel#pageTitle { font-size: 24px; font-weight: bold; color: #c41e3a; }"
        "QLabel#pageSub { font-size: 14px; color: #666666; }"
        "QLabel#previewLabel { font-size: 14px; color: #666; }"
        "QPushButton#themeBtn { background: white; color: #333; border: 2px solid #ddd; border-radius: 8px; "
        "padding: 14px 20px; font-size: 14px; min-width: 120px; }"
        "QPushButton#themeBtn:checked { border-color: #c41e3a; color: #c41e3a; background: #fff5f5; }"
        "QPushButton#themeBtn:hover { border-color: #c41e3a; }"
    );
    setStyleSheet(dlgStyle);

    // Content area
    m_stack->addWidget(createWelcomePage());
    m_stack->addWidget(createLicensePage());
    m_stack->addWidget(createThemePage());
    m_stack->addWidget(createVisualEffectsPage());
    m_stack->addWidget(createModelDetectionPage());
    m_stack->addWidget(createFeaturesPage());
    m_stack->addWidget(createCompletePage());

    mainLay->addWidget(m_stack, 1);

    // Bottom navigation bar
    QFrame *bottomBar = new QFrame();
    bottomBar->setFixedHeight(60);
    bottomBar->setStyleSheet("QFrame { background: white; border-top: 1px solid #e0e0e0; }");
    QHBoxLayout *bottomLay = new QHBoxLayout(bottomBar);
    bottomLay->setContentsMargins(24, 0, 24, 0);
    bottomLay->setSpacing(12);

    m_pageIndicator = new QLabel("1 / 7");
    m_pageIndicator->setAlignment(Qt::AlignCenter);
    m_pageIndicator->setStyleSheet("color: #999; font-size: 13px;");

    m_backBtn->setObjectName("backBtn");
    m_backBtn->setMinimumHeight(38);
    m_backBtn->setVisible(false); // Hidden on first page

    m_nextBtn->setMinimumHeight(38);

    bottomLay->addWidget(m_backBtn);
    bottomLay->addStretch();
    bottomLay->addWidget(m_pageIndicator);
    bottomLay->addStretch();
    bottomLay->addWidget(m_nextBtn);

    mainLay->addWidget(bottomBar);

    connect(m_backBtn, &QPushButton::clicked, this, &FirstRunDialog::goToPrevPage);
    connect(m_nextBtn, &QPushButton::clicked, this, &FirstRunDialog::goToNextPage);

    updateNavButtons();
}

/* ==================== Page Creators ==================== */

QWidget* FirstRunDialog::createWelcomePage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(40, 30, 40, 30);
    lay->setSpacing(20);

    QFrame *frame = new QFrame();
    frame->setObjectName("welcomeFrame");
    frame->setMinimumHeight(200);
    QVBoxLayout *fl = new QVBoxLayout(frame);
    fl->setContentsMargins(30, 30, 30, 30);

    QLabel *title = new QLabel("\u2605 国防安全科普教育软件");
    title->setObjectName("welcomeTitle");
    title->setAlignment(Qt::AlignCenter);
    fl->addWidget(title);

    QLabel *sub = new QLabel("v3.2.2 \u2022 制作者：傅琪");
    sub->setObjectName("welcomeSub");
    sub->setAlignment(Qt::AlignCenter);
    fl->addWidget(sub);

    QLabel *desc = new QLabel("欢迎使用国防安全科普教育软件！\n本软件将帮助您了解中国国防发展历程、装备知识，\n并提供 AI 智能问答与互动体验。");
    desc->setAlignment(Qt::AlignCenter);
    desc->setStyleSheet("color: rgba(255,255,255,0.85); font-size: 15px;");
    fl->addWidget(desc);

    lay->addWidget(frame);
    lay->addStretch();

    QLabel *hint = new QLabel("点击「下一步」开始初次设置");
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: #999; font-size: 13px;");
    lay->addWidget(hint);

    return page;
}

QWidget* FirstRunDialog::createLicensePage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(40, 30, 40, 30);
    lay->setSpacing(16);

    QLabel *title = new QLabel("\U0001f4dc 软件使用许可协议");
    title->setObjectName("pageTitle");
    lay->addWidget(title);

    QLabel *sub = new QLabel("请仔细阅读以下许可协议");
    sub->setObjectName("pageSub");
    lay->addWidget(sub);
    lay->addSpacing(10);

    QTextEdit *licenseText = new QTextEdit();
    licenseText->setReadOnly(true);
    licenseText->setHtml(
        "<h3 style='color:#c41e3a;'>国防安全科普教育软件 使用许可协议</h3>"
        "<p><b>版本：3.2.2</b></p>"
        "<hr>"
        "<p><b>一、软件说明</b></p>"
        "<p>本软件是一款国防安全科普教育工具，集成了国防装备展示、发展历程时间轴、"
        "AI 智能问答、AI 答题、知识问答、知识答题、激光防御模拟等功能。</p>"
        "<p><b>二、使用许可</b></p>"
        "<p>1. 本软件仅供教育和学习使用，不得用于任何商业目的。</p>"
        "<p>2. 用户可以免费复制、分发本软件用于非商业教育目的。</p>"
        "<p>3. 用户不得对本软件进行修改、反编译、反汇编或创建衍生作品。</p>"
        "<p>4. 本软件的著作权归作者所有，保留所有权利。</p>"
        "<p><b>三、免责声明</b></p>"
        "<p>1. 本软件按\"现状\"提供，不提供任何形式的明示或暗示保证。</p>"
        "<p>2. 作者不对因使用本软件而产生的任何直接或间接损失负责。</p>"
        "<p>3. 本软件中涉及的国防知识信息来源于公开资料，仅供参考。</p>"
        "<p><b>四、第三方组件</b></p>"
        "<p>本软件使用 Qt 框架（LGPL 许可）开发。Qt 不用于任何公司和商业用途。</p>"
        "<p><b>五、协议生效</b></p>"
        "<p>安装或使用本软件即表示您同意接受本协议的所有条款。</p>"
        "<hr>"
        "<p style='color:#888;'>制作者：傅琪</p>"
    );
    licenseText->setMinimumHeight(220);
    lay->addWidget(licenseText, 1);

    m_licenseCheck = new QCheckBox("我已阅读并同意此使用许可协议");
    m_licenseCheck->setStyleSheet("font-size: 14px; padding: 8px 0;");
    lay->addWidget(m_licenseCheck);

    return page;
}

QWidget* FirstRunDialog::createThemePage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(40, 30, 40, 30);
    lay->setSpacing(16);

    QLabel *title = new QLabel("\U0001f3a8 选择主题");
    title->setObjectName("pageTitle");
    lay->addWidget(title);

    QLabel *sub = new QLabel("选择您喜欢的界面主题风格");
    sub->setObjectName("pageSub");
    lay->addWidget(sub);
    lay->addSpacing(20);

    QHBoxLayout *btnLay = new QHBoxLayout();
    btnLay->setSpacing(16);

    m_themeGroup = new QButtonGroup(this);
    m_themeGroup->setExclusive(true);

    m_themeSystemBtn = new QPushButton("\U0001f4bb\n跟随系统");
    m_themeSystemBtn->setObjectName("themeBtn");
    m_themeSystemBtn->setCheckable(true);
    m_themeSystemBtn->setMinimumHeight(100);
    m_themeGroup->addButton(m_themeSystemBtn, 0);

    m_themeLightBtn = new QPushButton("\u2600\ufe0f\n浅色");
    m_themeLightBtn->setObjectName("themeBtn");
    m_themeLightBtn->setCheckable(true);
    m_themeLightBtn->setMinimumHeight(100);
    m_themeGroup->addButton(m_themeLightBtn, 1);

    m_themeDarkBtn = new QPushButton("\U0001f319\n深色");
    m_themeDarkBtn->setObjectName("themeBtn");
    m_themeDarkBtn->setCheckable(true);
    m_themeDarkBtn->setMinimumHeight(100);
    m_themeGroup->addButton(m_themeDarkBtn, 2);

    // Default: follow system
    if (systemDarkMode()) {
        m_themeDarkBtn->setChecked(true);
    } else {
        m_themeLightBtn->setChecked(true);
    }

    btnLay->addWidget(m_themeSystemBtn);
    btnLay->addWidget(m_themeLightBtn);
    btnLay->addWidget(m_themeDarkBtn);
    lay->addLayout(btnLay);
    lay->addStretch();

    QLabel *hint = new QLabel("您可以在软件设置中随时更改主题");
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: #999; font-size: 13px;");
    lay->addWidget(hint);

    return page;
}

QWidget* FirstRunDialog::createVisualEffectsPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(40, 30, 40, 30);
    lay->setSpacing(16);

    QLabel *title = new QLabel("\U0001f3a8 视觉效果设置");
    title->setObjectName("pageTitle");
    lay->addWidget(title);

    QLabel *sub = new QLabel("自定义界面视觉效果");
    sub->setObjectName("pageSub");
    lay->addWidget(sub);
    lay->addSpacing(20);

    // Acrylic effect option
    QFrame *visualFrame = new QFrame();
    visualFrame->setObjectName("featureCard");
    QVBoxLayout *vfLay = new QVBoxLayout(visualFrame);
    vfLay->setSpacing(12);

    m_acrylicCheck = new QCheckBox("\U0001f48e 启用亚克力效果（毛玻璃背景）");
    m_acrylicCheck->setStyleSheet("font-size: 14px; padding: 4px 0;");
    vfLay->addWidget(m_acrylicCheck);

    m_sidebarAutoCheck = new QCheckBox("\U0001f4cb 导航后自动收回侧边栏");
    m_sidebarAutoCheck->setChecked(true);
    m_sidebarAutoCheck->setStyleSheet("font-size: 14px; padding: 4px 0;");
    vfLay->addWidget(m_sidebarAutoCheck);

    lay->addWidget(visualFrame);

    lay->addSpacing(16);

    QLabel *preview = new QLabel("这些设置可在软件设置中随时调整");
    preview->setAlignment(Qt::AlignCenter);
    preview->setStyleSheet("color: #999; font-size: 13px;");
    lay->addWidget(preview);

    lay->addStretch();

    return page;
}

QWidget* FirstRunDialog::createModelDetectionPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(40, 30, 40, 30);
    lay->setSpacing(16);

    QLabel *title = new QLabel("\U0001f916 AI 服务设置");
    title->setObjectName("pageTitle");
    lay->addWidget(title);

    QLabel *sub = new QLabel("选择您使用的 AI 服务提供商");
    sub->setObjectName("pageSub");
    lay->addWidget(sub);
    lay->addSpacing(16);

    // 服务商选择
    lay->addWidget(new QLabel("AI 服务商:"));

    QHBoxLayout *providerLay = new QHBoxLayout();
    providerLay->setSpacing(12);

    m_ollamaProviderBtn = new QPushButton("\U0001f5a5\ufe0f  Ollama（本地）");
    m_ollamaProviderBtn->setCheckable(true);
    m_ollamaProviderBtn->setChecked(true);
    m_ollamaProviderBtn->setMinimumHeight(56);
    m_ollamaProviderBtn->setCursor(Qt::PointingHandCursor);

    m_deepseekProviderBtn = new QPushButton("\u2601\ufe0f  DeepSeek（云端）");
    m_deepseekProviderBtn->setCheckable(true);
    m_deepseekProviderBtn->setMinimumHeight(56);
    m_deepseekProviderBtn->setCursor(Qt::PointingHandCursor);

    providerLay->addWidget(m_ollamaProviderBtn, 1);
    providerLay->addWidget(m_deepseekProviderBtn, 1);
    lay->addLayout(providerLay);

    // 更新按钮样式的辅助函数
    auto updateProviderBtnStyles = [this]() {
        if (m_ollamaProviderBtn->isChecked()) {
            m_ollamaProviderBtn->setStyleSheet(
                "QPushButton { background: #c41e3a; color: white; border: 2px solid #c41e3a; "
                "border-radius: 8px; padding: 12px 16px; font-size: 14px; }"
                "QPushButton:hover { background: #e8294a; border-color: #e8294a; }");
            m_deepseekProviderBtn->setStyleSheet(
                "QPushButton { background: white; color: #333; border: 2px solid #ddd; "
                "border-radius: 8px; padding: 12px 16px; font-size: 14px; }"
                "QPushButton:hover { border-color: #c41e3a; }");
        } else {
            m_deepseekProviderBtn->setStyleSheet(
                "QPushButton { background: #c41e3a; color: white; border: 2px solid #c41e3a; "
                "border-radius: 8px; padding: 12px 16px; font-size: 14px; }"
                "QPushButton:hover { background: #e8294a; border-color: #e8294a; }");
            m_ollamaProviderBtn->setStyleSheet(
                "QPushButton { background: white; color: #333; border: 2px solid #ddd; "
                "border-radius: 8px; padding: 12px 16px; font-size: 14px; }"
                "QPushButton:hover { border-color: #c41e3a; }");
        }
    };

    // 使用 QSignalMapper 风格的连接
    connect(m_ollamaProviderBtn, &QPushButton::clicked, this, [this, updateProviderBtnStyles]() {
        if (!m_ollamaProviderBtn->isChecked()) {
            m_ollamaProviderBtn->setChecked(true);
            m_deepseekProviderBtn->setChecked(false);
        }
        updateProviderBtnStyles();
        // 切换服务商后立即重新检测模型
        if (m_stack->currentIndex() == 4) {
            startModelDetection();
        }
    });
    connect(m_deepseekProviderBtn, &QPushButton::clicked, this, [this, updateProviderBtnStyles]() {
        if (!m_deepseekProviderBtn->isChecked()) {
            m_deepseekProviderBtn->setChecked(true);
            m_ollamaProviderBtn->setChecked(false);
        }
        updateProviderBtnStyles();
        // 切换服务商后立即重新检测模型
        if (m_stack->currentIndex() == 4) {
            startModelDetection();
        }
    });

    // 初始化样式
    updateProviderBtnStyles();

    lay->addSpacing(10);

    // 模型检测状态和选择
    m_modelStatusLabel = new QLabel("请选择服务商后将自动检测可用模型");
    m_modelStatusLabel->setAlignment(Qt::AlignCenter);
    m_modelStatusLabel->setStyleSheet("font-size: 14px; color: #888;");
    lay->addWidget(m_modelStatusLabel);

    lay->addSpacing(8);
    lay->addWidget(new QLabel("选择模型:"));

    m_modelCombo = new QComboBox();
    m_modelCombo->setMinimumHeight(40);
    m_modelCombo->setEditable(false);
    m_modelCombo->setStyleSheet(
        "QComboBox { background: white; border: 1px solid #e0e0e0; border-radius: 8px; "
        "padding: 8px 12px; font-size: 14px; color: #333; }"
        "QComboBox QAbstractItemView { background: white; color: #333; }");
    m_modelCombo->addItem("等待检测...");
    m_modelCombo->setEnabled(false);
    lay->addWidget(m_modelCombo);

    m_modelHintLabel = new QLabel("选择服务商后将自动检测本地可用模型");
    m_modelHintLabel->setAlignment(Qt::AlignCenter);
    m_modelHintLabel->setStyleSheet("color: #999; font-size: 13px;");
    lay->addWidget(m_modelHintLabel);

    lay->addSpacing(10);

    // 跳过 AI 设置按钮
    m_skipAiBtn = new QPushButton("跳过 AI 设置");
    m_skipAiBtn->setObjectName("skipAiBtn");
    m_skipAiBtn->setStyleSheet(
        "QPushButton#skipAiBtn { background: transparent; color: #999; border: 1px dashed #ccc; "
        "padding: 10px 24px; border-radius: 6px; font-size: 13px; }"
        "QPushButton#skipAiBtn:hover { background: #f5f5f5; color: #666; }");
    connect(m_skipAiBtn, &QPushButton::clicked, this, [this]() {
        int ret = QMessageBox::question(this, "跳过 AI 设置",
            "跳过将无法使用 AI 问答和 AI 答题功能。\n\n"
            "您仍可在软件设置中重新启用 AI 功能。\n\n是否确认跳过？",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            m_aiSkipped = true;
            // 直接跳到功能介绍页（跳过当前页）
            m_stack->setCurrentIndex(5);
            updateNavButtons();
        }
    });
    lay->addWidget(m_skipAiBtn, 0, Qt::AlignCenter);

    lay->addStretch();

    return page;
}

QWidget* FirstRunDialog::createFeaturesPage()
{
    QWidget *page = new QWidget();
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    QWidget *content = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(content);
    lay->setContentsMargins(40, 20, 40, 20);
    lay->setSpacing(12);

    QLabel *title = new QLabel("\U0001f4cb 软件功能介绍");
    title->setObjectName("pageTitle");
    lay->addWidget(title);

    QLabel *sub = new QLabel("了解国防安全科普教育软件的各项功能");
    sub->setObjectName("pageSub");
    lay->addWidget(sub);
    lay->addSpacing(10);

    struct Feature { QString icon; QString title; QString desc; };
    QList<Feature> features = {
        {"\U0001f6e1\ufe0f", "国防装备展示", "展示中国自主研发的九大国防装备，包括歼-20、福建舰、东风-41等，点击卡片可查看详细参数与介绍。"},
        {"\U0001f4c5", "发展历程时间轴", "数轴式时间轴设计，拖动浏览从南昌起义到未来愿景的完整历程，中心放大、边缘缩小的视觉层次效果。"},
        {"\U0001f916", "AI 智能问答", "集成 Ollama 本地大模型或 DeepSeek 云端模型，启动时自动检测可用模型，在设置中选择模型，预制国防知识专属提示词，流式响应实时显示。"},
        {"\U0001f4dd", "AI 智能答题", "AI 自动生成国防知识选择题，可选3/5/10题，即时判分与答案解析，答题完成评级。"},
        {"\U0001f4d6", "知识问答", "预制12道国防知识问答，涵盖国防政策、兵役制度、航母发展等核心知识点。"},
        {"\U0001f3af", "知识答题", "预制20道国防知识选择题，测试您的国防知识水平，即时反馈与评分。"},
        {"\U0001f3ae", "激光防御模拟", "2D炮塔防御小游戏：炮管朝向鼠标，按住鼠标持续发射激光，消灭从边缘逼近的无人机。"},
        {"\u2699\ufe0f", "软件设置", "支持跟随系统/深色/浅色主题，字体大小调整，Ollama 服务地址、模型名称自定义。"}
    };

    for (const auto &f : features) {
        QFrame *card = new QFrame();
        card->setObjectName("featureCard");
        QHBoxLayout *cl = new QHBoxLayout(card);
        cl->setSpacing(14);

        QLabel *icon = new QLabel(f.icon);
        icon->setStyleSheet("font-size: 28px;");
        icon->setFixedWidth(40);

        QVBoxLayout *textL = new QVBoxLayout();
        textL->setSpacing(4);
        QLabel *ft = new QLabel(f.title);
        ft->setObjectName("featureTitle");
        QLabel *fd = new QLabel(f.desc);
        fd->setObjectName("featureDesc");
        fd->setWordWrap(true);
        textL->addWidget(ft);
        textL->addWidget(fd);

        cl->addWidget(icon);
        cl->addLayout(textL);
        lay->addWidget(card);
    }

    lay->addStretch();
    scroll->setWidget(content);

    QVBoxLayout *pageLay = new QVBoxLayout(page);
    pageLay->setContentsMargins(0, 0, 0, 0);
    pageLay->addWidget(scroll);

    return page;
}

QWidget* FirstRunDialog::createCompletePage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(40, 30, 40, 30);
    lay->setSpacing(20);

    QLabel *title = new QLabel("\U0001f389 设置完成！");
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter);
    lay->addWidget(title);

    lay->addSpacing(10);

    QString aiStatus = m_aiSkipped ? "\u26a0\ufe0f AI 功能已跳过（可在设置中启用）" : "\u2714 AI 模型已设置";
    QLabel *msg = new QLabel(
        QString("恭喜！您已经完成国防安全科普教育软件的初次设置。\n\n"
        "\u2714 主题已设置\n"
        "\u2714 视觉效果已设置\n"
        "\u2714 已阅读并同意使用许可协议\n"
        "%1\n\n"
        "现在点击「开始使用」即可进入软件主界面。").arg(aiStatus)
    );
    msg->setAlignment(Qt::AlignCenter);
    msg->setStyleSheet("font-size: 15px; color: #555; line-height: 1.8;");
    lay->addWidget(msg);

    lay->addStretch();

    QLabel *hint = new QLabel("祝您使用愉快！");
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: #c41e3a; font-size: 16px; font-weight: bold;");
    lay->addWidget(hint);

    return page;
}

/* ==================== Navigation ==================== */

void FirstRunDialog::updateNavButtons()
{
    int page = m_stack->currentIndex();
    int total = m_stack->count();
    m_pageIndicator->setText(QString("%1 / %2").arg(page + 1).arg(total));

    // Show back button only on pages after first
    m_backBtn->setVisible(page > 0);
    m_backBtn->setEnabled(page > 0);

    if (page == total - 1) {
        m_nextBtn->setText("开始使用");
    } else if (page == 0) {
        m_nextBtn->setText("下一步 >");
    } else {
        m_nextBtn->setText("下一步 >");
    }
}

void FirstRunDialog::goToPrevPage()
{
    int page = m_stack->currentIndex();
    if (page > 0) {
        m_stack->setCurrentIndex(page - 1);
        updateNavButtons();
        // 离开许可协议页时重置复选框样式
        if (page == 1) {
            m_licenseCheck->setStyleSheet("font-size: 14px; padding: 8px 0;");
        }
    }
}

void FirstRunDialog::goToNextPage()
{
    int page = m_stack->currentIndex();
    int total = m_stack->count();

    // 验证许可协议页面
    if (page == 1 && !m_licenseCheck->isChecked()) {
        m_licenseCheck->setStyleSheet("color: #ff4444; font-size: 14px; padding: 8px 0;");
        return;
    }
    // 重置许可协议复选框样式（如果之前验证失败变红）
    if (page == 1 && m_licenseCheck->isChecked()) {
        m_licenseCheck->setStyleSheet("font-size: 14px; padding: 8px 0;");
    }

    if (page < total - 1) {
        m_stack->setCurrentIndex(page + 1);
        updateNavButtons();

        // 进入模型检测页时立即开始检测
        if (m_stack->currentIndex() == 4 && !m_aiSkipped) {
            startModelDetection();
        }
    } else {
        finishSetup();
    }
}

void FirstRunDialog::finishSetup()
{
    QString settingsPath = SettingsManager::defaultSettingsPath();
    QDir().mkpath(QFileInfo(settingsPath).absolutePath());
    QSettings settings(settingsPath, QSettings::IniFormat);

    // Save theme
    settings.setValue("theme", theme());

    // Save visual effects settings
    settings.setValue("acrylicMode", m_acrylicCheck ? m_acrylicCheck->isChecked() : false);
    settings.setValue("sidebarAutoCollapse", m_sidebarAutoCheck ? m_sidebarAutoCheck->isChecked() : true);

    if (m_aiSkipped) {
        // AI skipped, just mark as no model
        settings.setValue("ollamaModel", "");
        settings.setValue("deepseekModel", "");
    } else {
        // Save provider and model
        bool isOllama = m_ollamaProviderBtn->isChecked();
        settings.setValue("aiProvider", isOllama ? "ollama" : "deepseek");

        QString selectedModel = m_modelCombo->currentText();
        if (!selectedModel.isEmpty() && selectedModel != "等待检测..." && selectedModel != "未找到可用模型") {
            if (isOllama) {
                settings.setValue("ollamaModel", selectedModel);
                settings.setValue("discoveredOllamaModels", m_detectedModels);
            } else {
                settings.setValue("deepseekModel", selectedModel);
                settings.setValue("discoveredDeepseekModels", m_detectedModels);
            }
        }
    }
    settings.sync();

    accept();
}

/* ==================== Model Detection ==================== */

void FirstRunDialog::startModelDetection()
{
    bool isOllama = m_ollamaProviderBtn->isChecked();

    if (isOllama) {
        // Ollama - detect local models via /api/tags
        m_modelStatusLabel->setText("正在检测 Ollama 本地服务...");
        m_modelCombo->clear();
        m_modelCombo->addItem("检测中...");
        m_modelCombo->setEnabled(false);
        m_modelHintLabel->setText("通过 ollama list 命令检测本地模型");

        QString ollamaUrl = "http://localhost:11434";
        QUrl ollamaApiUrl(ollamaUrl + "/api/tags");
        QNetworkRequest ollamaRequest(ollamaApiUrl);
        ollamaRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply *ollamaReply = m_netManager->get(ollamaRequest);
        // 5 second timeout
        QTimer::singleShot(5000, ollamaReply, [ollamaReply]() {
            if (ollamaReply->isRunning()) {
                ollamaReply->abort();
            }
        });

        connect(ollamaReply, &QNetworkReply::finished, this, [this, ollamaReply]() {
            onOllamaDetectionFinished();
            ollamaReply->deleteLater();
        });
    } else {
        // DeepSeek - just show the pre-defined cloud models
        m_modelStatusLabel->setText("DeepSeek 云端模型（需要 API 密钥）");
        m_modelCombo->clear();
        m_modelCombo->addItem("deepseek-chat");
        m_modelCombo->addItem("deepseek-coder");
        m_modelCombo->setEnabled(true);
        m_modelCombo->setCurrentIndex(0);
        m_modelHintLabel->setText("请在软件设置中配置 DeepSeek API 密钥");
        m_detectedModels = {"deepseek-chat", "deepseek-coder"};
    }
}

void FirstRunDialog::onOllamaDetectionFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    m_detectedModels.clear();
    bool ollamaOk = false;
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        if (obj.contains("models")) {
            for (const QJsonValue &v : obj.value("models").toArray()) {
                QString name = v.toObject().value("name").toString();
                if (!name.isEmpty()) m_detectedModels.append(name);
            }
            if (!m_detectedModels.isEmpty()) ollamaOk = true;
        }
    }

    if (ollamaOk) {
        m_modelStatusLabel->setText(QString("检测到 %1 个本地模型").arg(m_detectedModels.size()));
        m_modelCombo->clear();
        for (const QString &m : m_detectedModels) {
            m_modelCombo->addItem(m);
        }
        m_modelCombo->setEnabled(true);
        m_modelCombo->setCurrentIndex(0);
        m_modelHintLabel->setText("已自动选择第一个模型，可在设置中更改");
    } else {
        m_modelStatusLabel->setText("未检测到 Ollama 服务或未找到模型");
        m_modelCombo->clear();
        m_modelCombo->addItem("未找到可用模型");
        m_modelCombo->setEnabled(false);
        m_modelHintLabel->setText("请确保 Ollama 已启动并下载了模型（运行 ollama list 检查）");
    }
}

/* ==================== Getters ==================== */

QString FirstRunDialog::theme() const
{
    if (m_themeGroup) {
        int id = m_themeGroup->checkedId();
        if (id == 2) return "dark";
        if (id == 1) return "light";
    }
    return "system";
}

bool FirstRunDialog::licenseAccepted() const
{
    return m_licenseCheck ? m_licenseCheck->isChecked() : false;
}
