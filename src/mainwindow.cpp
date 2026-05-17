#include "mainwindow.h"
#include "tutorialdialog.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QScrollBar>
#include <QKeyEvent>
#include <QTimer>
#include <QDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QGraphicsOpacityEffect>
#ifdef Q_OS_WIN
#include <windows.h>

// Win10+ Acrylic blur API structures
struct ACCENTPOLICY {
    int AccentState;
    int AccentFlags;
    int GradientColor;
    int AnimationId;
};

struct WCA_DATA {
    int Attrib;
    PVOID pvData;
    SIZE_T cbData;
};
#endif

/* 辅助函数：检测系统是否处于深色模式 */
static bool systemDarkMode()
{
    // Windows 10+ 注册表键
#ifdef Q_OS_WIN
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                  QSettings::NativeFormat);
    return reg.value("AppsUseLightTheme", 1).toInt() == 0;
#else
    return false;
#endif
}

/* ================================================================== */
/*  主题颜色                                                          */
/* ================================================================== */
struct Theme {
    QString bg, surface, text, text2, border, hover, primary, primaryLight, accent;
};
static const Theme DARK = {
    "#000000", "#111111", "#ffffff", "#888888",
    "#333333", "#1a1a1a", "#c41e3a", "#e8294a", "#ffd700"
};
static const Theme LIGHT = {
    "#f0f0f0", "#ffffff", "#1a1a1a", "#666666",
    "#dddddd", "#e8e8e8", "#c41e3a", "#e8294a", "#b8860b"
};

/* ================================================================== */
/*  构造函数                                                          */
/* ================================================================== */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_darkMode(true)
    , m_themeMode("system")
    , m_acrylicMode(false)
    , m_sidebarAutoCollapse(true)
    , m_menuBtn(nullptr)
    , m_topTitle(nullptr)
    , m_drawer(nullptr)
    , m_drawerCloseBtn(nullptr)
    , m_drawerOpen(false)
    , m_stack(new QStackedWidget(this))
    , m_timelineWidget(nullptr)
    , m_statusDot(new QLabel(this))
    , m_statusLabel(new QLabel(this))
    , m_chatDisplay(new QTextEdit(this))
    , m_chatInput(new QTextEdit(this))
    , m_quizDifficulty(new QComboBox(this))
    , m_quizProgress(new QLabel(this))
    , m_quizQuestion(new QTextEdit(this))
    , m_quizFeedback(new QLabel(this))
    , m_quizScoreLabel(new QLabel(this))
    , m_quizCurrent(0), m_quizCorrect(0), m_quizTotal(5), m_quizSelectedAnswer(-1)
    , m_quizNextBtn(nullptr)
    , m_quizReceiving(false)
    , m_kQuizProgress(new QLabel(this))
    , m_kQuizQuestion(new QTextEdit(this))
    , m_kQuizFeedback(new QLabel(this))
    , m_kQuizScoreLabel(new QLabel(this))
    , m_kQuizNextBtn(nullptr)
    , m_kQuizCurrent(0), m_kQuizCorrect(0), m_kQuizTotal(0), m_kQuizSelectedAnswer(-1)
    , m_urlInput(new QLineEdit(this))
    , m_modelCombo(new QComboBox(this))
    , m_modelSelectCombo(new QComboBox(this))
    , m_promptInput(new QTextEdit(this))
    , m_darkBtn(new QPushButton(this))
    , m_lightBtn(new QPushButton(this))
    , m_autoBtn(new QPushButton(this))
    , m_aiServiceManager(new AiServiceManager(this))
    , m_startupNetManager(new QNetworkAccessManager(this))
    , m_settingsManager(new SettingsManager(this))
    , m_saveDebounceTimer(new QTimer(this))
    , m_isReceiving(false)
    , m_aiProviderCombo(nullptr)
    , m_aiSettingsStack(nullptr)
    , m_deepseekApiKeyInput(nullptr)
    , m_deepseekApiUrlInput(nullptr)
{
    setWindowTitle("国防安全科普教育软件");
    setMinimumSize(900, 600);
    resize(1400, 900);

    initEquipmentData();
    setupUi();            // 先创建UI组件
    loadSettingsToUi();   // 再加载设置到UI
    applyTheme();

    // 设置防抖定时器: 500ms 内多次触发只保存一次
    m_saveDebounceTimer->setSingleShot(true);
    m_saveDebounceTimer->setInterval(500);
    connect(m_saveDebounceTimer, &QTimer::timeout, this, &MainWindow::saveSettings);

    connect(m_aiServiceManager, &AiServiceManager::statusChanged,   this, &MainWindow::onAiServiceStatusChanged);
    connect(m_aiServiceManager, &AiServiceManager::responseChunk,   this, &MainWindow::onResponseChunk);
    connect(m_aiServiceManager, &AiServiceManager::responseReceived,this, &MainWindow::onResponseReceived);
    connect(m_aiServiceManager, &AiServiceManager::modelsFetched,   this, &MainWindow::onModelsFetched);
    connect(m_aiServiceManager, &AiServiceManager::errorOccurred,   this, &MainWindow::onErrorOccurred);

    // 启动时静默检测模型并自动连接（强制开启）
    QTimer::singleShot(800, this, [this](){
        startupModelDetection();
        checkAiService();
    });
}

MainWindow::~MainWindow(){}

/* ================================================================== */
/*  主题 — 通过 qApp 样式表的单一事实源                  */
/* ================================================================== */
void MainWindow::applyTheme()
{
    const Theme &t = m_darkMode ? DARK : LIGHT;

    // Acrylic mode: use semi-transparent backgrounds for a frosted glass effect
    QString surfaceBg = m_acrylicMode
        ? (m_darkMode ? "rgba(17,17,17,0.85)" : "rgba(255,255,255,0.85)")
        : t.surface;

    QString drawerBg = m_acrylicMode
        ? (m_darkMode ? "rgba(17,17,17,0.9)" : "rgba(255,255,255,0.9)")
        : t.surface;

    QString topBarBg = m_acrylicMode
        ? (m_darkMode ? "rgba(17,17,17,0.92)" : "rgba(255,255,255,0.92)")
        : t.surface;

    QString ss = QString(
        /* ---- 全局基础 ---- */
        "QMainWindow, QWidget#centralWidget { background:%1; color:%2; }"
        "QLabel { color:%2; background:transparent; border:none; }"
        "QCheckBox { color:%2; background:transparent; }"
        "QComboBox { color:%2; background:%3; border:1px solid %5; padding:6px 10px; border-radius:6px; }"
        "QComboBox QAbstractItemView { background:%3; color:%2; selection-background-color:%7; border:none; padding:4px; }"
        "QComboBox::drop-down { border:none; }"

        /* ---- 滚动区域 ---- */
        "QScrollArea { border:none; background:transparent; }"
        "QScrollArea > QWidget > QWidget { background:%1; }"

        /* ---- 现代滚动条 ---- */
        "QScrollBar:vertical { background:transparent; width:8px; margin:0; }"
        "QScrollBar::handle:vertical { background:rgba(128,128,128,0.35); min-height:30px; border-radius:4px; }"
        "QScrollBar::handle:vertical:hover { background:rgba(128,128,128,0.55); }"
        "QScrollBar::handle:vertical:pressed { background:rgba(128,128,128,0.75); }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; background:none; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background:none; }"
        "QScrollBar:horizontal { background:transparent; height:8px; margin:0; }"
        "QScrollBar::handle:horizontal { background:rgba(128,128,128,0.35); min-width:30px; border-radius:4px; }"
        "QScrollBar::handle:horizontal:hover { background:rgba(128,128,128,0.55); }"
        "QScrollBar::handle:horizontal:pressed { background:rgba(128,128,128,0.75); }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width:0; background:none; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background:none; }"

        /* ---- 文本编辑 ---- */
        "QTextEdit { background:%3; color:%2; border:1px solid %5; border-radius:10px; padding:14px; }"
        "QTextEdit#chatDisplay { font-size:14px; line-height:1.6; }"
        "QTextEdit#chatInput { border-radius:8px; padding:10px; font-size:14px; }"
        "QTextEdit#chatInput:focus, QTextEdit#promptInput:focus { border-color:%7; }"
        "QTextEdit#quizQuestion { font-size:15px; padding:16px; }"
        "QTextEdit#promptInput { border-radius:8px; padding:9px 12px; font-size:13px; }"

        /* ---- 行编辑 ---- */
        "QLineEdit { background:%3; color:%2; border:1px solid %5; padding:9px 12px; border-radius:8px; }"
        "QLineEdit:focus { border-color:%7; }"

        /* ---- 滑块 ---- */
        "QSlider::groove:horizontal { height:6px; background:%5; border-radius:3px; }"
        "QSlider::handle:horizontal { width:16px; height:16px; margin:-6px 0; background:%7; border-radius:8px; border:2px solid %3; }"
        "QSlider::handle:horizontal:hover { background:%8; }"

        /* ---- 顶部栏 ---- */
        "QFrame#topBar { background:" + topBarBg + "; border-bottom:1px solid %5; }"

        /* ---- 侧边栏 ---- */
        "QFrame#drawer { background:" + drawerBg + "; border-right:1px solid %5; }"
        "QPushButton#drawerClose { color:%2; background:transparent; border:none; font-size:20px; border-radius:6px; }"
        "QPushButton#drawerClose:hover { background:%6; }"
        "QPushButton#hamburgerBtn { color:%2; background:transparent; border:none; font-size:24px; border-radius:8px; }"
        "QPushButton#hamburgerBtn:hover { background:%6; }"

        /* ---- 导航按钮 ---- */
        "QPushButton#navBtn { background:transparent; color:%2; border:none; font-size:15px; text-align:left; padding-left:24px; border-radius:6px; margin:2px 8px; }"
        "QPushButton#navBtn:hover { background:%6; }"
        "QPushButton#navBtn[active=\"true\"] { background:rgba(196,30,58,0.2); color:%8; font-weight:bold; }"

        /* ---- 标签类型 ---- */
        "QLabel#heroTitle { font-size:40px; font-weight:bold; color:%9; }"
        "QLabel#sectionTitle { font-size:22px; font-weight:bold; color:%9; }"
        "QLabel#pageTitle { font-size:30px; font-weight:bold; color:%9; }"
        "QLabel#topTitle { font-size:18px; font-weight:bold; color:%9; }"
        "QLabel#drawerLogo { font-size:18px; font-weight:bold; color:%9; }"
        "QLabel#subLabel { font-size:14px; color:%4; }"
        "QLabel#dimLabel { font-size:12px; color:%4; }"
        "QLabel#versionLabel { font-size:12px; color:%4; }"
        "QLabel#equipName { font-size:17px; font-weight:bold; color:%9; }"
        "QLabel#equipDesc { font-size:13px; color:%4; line-height:1.5; }"
        "QLabel#equipLink { font-size:12px; }"
        "QLabel#catBadge { font-size:11px; background:rgba(128,128,128,0.15); padding:3px 10px; border-radius:8px; }"
        "QLabel#yearLabel { font-size:20px; font-weight:bold; color:%9; }"
        "QLabel#titleLabel { font-size:14px; color:%2; }"
        "QLabel#periodBadge { font-size:11px; background:rgba(128,128,128,0.1); padding:3px 8px; border-radius:6px; }"
        "QLabel#fontSizeLabel { color:%4; }"
        "QLabel#feedbackLabel { font-size:14px; }"

        /* ---- 状态 ---- */
        "QLabel#statusDot { border-radius:5px; }"
        "QLabel#statusDot[status=\"connected\"] { background:#00dd77; }"
        "QLabel#statusDot[status=\"disconnected\"] { background:#ff5555; }"
        "QLabel#statusLabel { font-size:13px; }"
        "QLabel#statusLabel[status=\"connected\"] { color:#00dd77; }"
        "QLabel#statusLabel[status=\"disconnected\"] { color:#ff5555; }"

        /* ---- 答题标签 ---- */
        "QLabel#quizProgress { font-size:13px; color:%4; }"
        "QLabel#quizScoreLabel { font-size:14px; color:%9; }"

        /* ---- 分隔线 ---- */
        "QFrame#separator { background:%5; border:none; }"

        /* ---- 装备卡片 ---- */
        "QFrame#equipCard { background:" + surfaceBg + "; border:1px solid %5; border-radius:14px; padding:20px; }"
        "QFrame#equipCard:hover { border-color:%9; background:" + (m_acrylicMode ? (m_darkMode ? "rgba(30,30,30,0.9)" : "rgba(245,245,245,0.9)") : "rgba(255,255,255,0.03)") + "; }"

        /* ---- 时间轴项目 ---- */
        "QFrame#timelineItem { background:%3; border-radius:10px; padding:14px 18px; margin:4px 0; }"
        "QFrame#timelinePast   { border-left:4px solid #888; }"
        "QFrame#timelinePresent { border-left:4px solid %7; }"
        "QFrame#timelineFuture  { border-left:4px solid %9; }"

        /* ---- 聊天状态栏 ---- */
        "QFrame#statusBar { background:%3; border:1px solid %5; border-radius:8px; padding:8px 14px; }"

        /* ---- 答题配置栏 ---- */
        "QFrame#quizCfg { background:%3; border:1px solid %5; border-radius:8px; padding:8px 14px; }"

        /* ---- 答题选项按钮 ---- */
        "QPushButton#ansBtn { background:%3; border:1px solid %5; color:%2; text-align:left; padding:10px 16px; border-radius:8px; font-size:14px; }"
        "QPushButton#ansBtn:hover { border-color:%7; background:%6; }"
        "QPushButton#ansBtn[correct=\"true\"] { background:rgba(0,220,100,0.15); border:2px solid #00dd77; color:#00dd77; font-weight:bold; }"
        "QPushButton#ansBtn[wrong=\"true\"] { background:rgba(255,70,70,0.15); border:2px solid #ff5555; color:#ff5555; font-weight:bold; }"
        "QPushButton#ansBtn[dim=\"true\"] { color:%4; opacity:0.6; }"

        /* ---- 答题下一题按钮 ---- */
        "QPushButton#quizNextBtn { background:%7; color:white; border:none; padding:10px 28px; border-radius:8px; font-size:14px; }"
        "QPushButton#quizNextBtn:hover { background:%8; }"

        /* ---- 设置区域框架 ---- */
        "QFrame#sectionFrame { background:" + surfaceBg + "; border:1px solid %5; border-radius:12px; padding:20px; margin:6px 0; }"

        /* ---- 主题切换按钮 ---- */
        "QPushButton#themeToggleBtn { background:%3; border:2px solid %5; color:%2; border-radius:8px; font-size:13px; padding: 6px 12px; }"
        "QPushButton#themeToggleBtn:checked { border-color:%7; color:%7; background:rgba(196,30,58,0.1); }"
        "QPushButton#themeToggleBtn:hover { border-color:%7; }"

        /* ---- 主要/操作按钮 ---- */
        "QPushButton#primaryBtn { background:%7; color:white; border:none; border-radius:8px; font-size:14px; }"
        "QPushButton#primaryBtn:hover { background:%8; }"
        "QPushButton#resetBtn { background:%3; border:1px solid %5; color:%2; border-radius:8px; font-size:13px; }"
        "QPushButton#resetBtn:hover { background:%6; border-color:%7; }"

        /* ---- 快捷提问按钮 ---- */
        "QPushButton#quickBtn { background:%3; border:1px solid %5; color:%2; border-radius:16px; padding:6px 14px; font-size:12px; }"
        "QPushButton#quickBtn:hover { background:%7; color:white; border-color:%7; }"

        /* ---- 参数框架 ---- */
        "QFrame#specFrame { background:%3; border:1px solid %5; border-radius:10px; padding:14px; }"

        /* ---- 对话框 ---- */
        "QDialog { background:%1; color:%2; }"

        /* ---- 游戏栏 ---- */
        "QFrame#gameBar { background:%3; border-top:1px solid %5; }"
    ).arg(t.bg,      // %1
          t.text,     // %2
          t.surface,  // %3
          t.text2,    // %4
          t.border,   // %5
          t.hover,    // %6
          t.primary,  // %7
          t.primaryLight, // %8
          t.accent);  // %9

    qApp->setStyleSheet(ss);

    // Note: Windows native acrylic blur API removed as it was causing
    // the title bar to disappear. Qt semi-transparent backgrounds are sufficient.

    // 更新时间轴组件深色模式
    if (m_timelineWidget) m_timelineWidget->setDarkMode(m_darkMode);

    // 更新游戏组件深色模式
    if (m_gameWidget) m_gameWidget->setDarkMode(m_darkMode);

    // 只更新需要主题更新的按钮
    for (auto btn : m_navButtons) {
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}

/* ================================================================== */
/*  装备数据                                                          */
/* ================================================================== */
void MainWindow::initEquipmentData()
{
    m_equipmentList = {
        {"j20","歼-20 隐身战斗机","空军",
         "中国第五代隐身战斗机",
         "歼-20（绰号：威龙）是中国自主研制的单座双发第五代战斗机，采用鸭式气动布局，"
         "配备涡扇-15发动机，具备超音速巡航与高态势感知能力。2017年正式列装，"
         "标志着中国成为继美国后第二个自主装备五代机的国家。",
         "类型: 第五代隐身战斗机\n长度: 约20.3米\n最大速度: 约2.5马赫\n作战半径: 约2000公里\n服役: 2017年"},

        {"fujian","福建舰 航空母舰","海军",
         "中国首艘电磁弹射型航空母舰",
         "福建舰（舷号18）是中国完全自主设计的首艘弹射型航母，满载排水量约8万吨，"
         "配置电磁弹射与阻拦装置，可起降歼-35等重型舰载机，"
         "2022年6月下水，实现了从滑跃到弹射的跨越。",
         "类型: 电磁弹射型航母\n满载排水量: 约8万吨\n弹射方式: 电磁弹射\n舰载机: 歼-35、空警-600\n下水: 2022年6月"},

        {"df41","东风-41 洲际弹道导弹","火箭军",
         "新一代陆基机动洲际弹道导弹",
         "东风-41采用三级固体燃料发动机，射程超14000公里，可携带MIRV多弹头，"
         "公路机动发射，反应时间短、突防能力强。2019年国庆阅兵首次亮相。",
         "类型: 洲际弹道导弹\n射程: 超过14000公里\n弹头: MIRV多弹头\n推进: 三级固体燃料\n亮相: 2019年"},

        {"type99a","99A 主战坦克","陆军",
         "中国新一代数字化主战坦克",
         "99A搭载125mm滑膛炮，可发射炮射导弹，配备复合装甲与反应装甲，"
         "安装综合信息化作战系统，运动中首发精准命中目标。"
         "标志中国陆军从机械化向信息化的跨越。",
         "类型: 主战坦克\n战斗全重: 约55吨\n主炮: 125毫米滑膛炮\n发动机: 1500马力\n最大速度: 约70km/h"},

        {"type055","055型 驱逐舰","海军",
         "万吨级导弹驱逐舰",
         "055型满载排水量超12000吨，112个垂直发射单元，装备双波段有源相控阵雷达，"
         "可发射鹰击-18/20、海红旗-9B等多种导弹，"
         "是世界上最先进的驱逐舰之一。",
         "类型: 导弹驱逐舰\n满载排水量: 约12500吨\n垂发单元: 112个\n雷达: 双波段相控阵\n首舰: 南昌舰(101"},

        {"hq9","红旗-9B 防空导弹","空军/陆军",
         "新一代远程地空导弹系统",
         "红旗-9B采用主动雷达末制导，可拦截飞机、巡航导弹和战术弹道导弹，"
         "具备抗饱和攻击和全天候作战能力，是国土防空体系核心装备。",
         "类型: 远程地空导弹\n最大射程: 约300公里\n最大射高: 约30公里\n制导: 主动雷达末制导\n发射: 垂直发射"},

        {"kj500","空警-500 预警机","空军",
         "新一代中型预警指挥机",
         "空警-500以运-9为平台，搭载数字阵列有源相控阵雷达，"
         "探测距离约470公里，可同时跟踪数百个目标，是空军体系作战核心节点。",
         "类型: 中型预警指挥机\n平台: 运-9\n雷达: 数字阵列相控阵\n探测距离: 约470km\n服役: 2015年"},

        {"y20","运-20 大型运输机","空军",
         "中国自主研制的大型军用运输机",
         "运-20（鲲鹏）最大载重约66吨，可运送主战坦克等大型装备，"
         "具备短距和高原起降能力。运油-20空中加油型已服役。",
         "类型: 大型军用运输机\n最大载重: 约66吨\n航程: 约7800km\n发动机: 涡扇-20\n服役: 2016年"},

        {"df17","东风-17 高超音速导弹","火箭军",
         "世界首款实战化高超音速武器",
         "东风-17采用助推-滑翔模式，速度超10马赫，弹道不可预测，"
         "现有反导系统难以拦截。2019年国庆阅兵首秀。",
         "类型: 高超音速滑翔导弹\n最大速度: 超10马赫\n射程: 约1800-2500km\n弹头: 乘波体\n亮相: 2019年"}
    };
}

/* ================================================================== */
/*  UI 构建                                                           */
/* ================================================================== */
void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    central->setObjectName("centralWidget");
    QVBoxLayout *mainLay = new QVBoxLayout(central);
    mainLay->setContentsMargins(0,0,0,0);
    mainLay->setSpacing(0);

    // ---- 顶部栏 ----
    QFrame *topBar = new QFrame();
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(52);
    QHBoxLayout *topLay = new QHBoxLayout(topBar);
    topLay->setContentsMargins(12,0,12,0);

    m_menuBtn = new QPushButton("☰");
    m_menuBtn->setFixedSize(42, 42);
    m_menuBtn->setObjectName("hamburgerBtn");
    connect(m_menuBtn, &QPushButton::clicked, this, &MainWindow::toggleSidebar);
    topLay->addWidget(m_menuBtn);

    m_topTitle = new QLabel("国防装备");
    m_topTitle->setObjectName("topTitle");
    topLay->addWidget(m_topTitle);
    topLay->addStretch();
    mainLay->addWidget(topBar);

    // ---- 底部区域：侧边栏 + 内容（推挤布局） ----
    QHBoxLayout *bodyLay = new QHBoxLayout();
    bodyLay->setContentsMargins(0,0,0,0);
    bodyLay->setSpacing(0);

    // 侧边栏
    m_drawer = new QFrame();
    m_drawer->setObjectName("drawer");
    m_drawer->setFixedWidth(0);  // 初始收起

    QVBoxLayout *drawerLay = new QVBoxLayout(m_drawer);
    drawerLay->setContentsMargins(0,0,0,0);
    drawerLay->setSpacing(0);

    // 侧边栏头部
    QWidget *drawerHeader = new QWidget();
    drawerHeader->setFixedHeight(52);
    QHBoxLayout *dhLay = new QHBoxLayout(drawerHeader);
    dhLay->setContentsMargins(16,0,8,0);

    QLabel *logo = new QLabel("★ 国防科普");
    logo->setObjectName("drawerLogo");
    dhLay->addWidget(logo);
    dhLay->addStretch();

    m_drawerCloseBtn = new QPushButton("◀");
    m_drawerCloseBtn->setFixedSize(36, 36);
    m_drawerCloseBtn->setObjectName("drawerClose");
    connect(m_drawerCloseBtn, &QPushButton::clicked, this, &MainWindow::closeSidebar);
    dhLay->addWidget(m_drawerCloseBtn);
    drawerLay->addWidget(drawerHeader);

    QFrame *sep = new QFrame(); sep->setFixedHeight(1);
    sep->setObjectName("separator");
    drawerLay->addWidget(sep);

    struct NavItem { QString icon; QString text; };
    QList<NavItem> navItems = {
        {"🏠","首  页"},{"📅","时间轴"},{"💬","AI 问答"},{"📝","AI 答题"},{"📖","知识问答"},{"🎯","知识答题"},{"🎮","模  拟"},{"⚙️","设  置"}
    };
    for (int i = 0; i < navItems.size(); ++i) {
        QPushButton *btn = new QPushButton(navItems[i].icon + "  " + navItems[i].text);
        btn->setFixedHeight(50);
        btn->setObjectName("navBtn");
        int idx = i;
        connect(btn, &QPushButton::clicked, this, [this, idx](){
            if (m_sidebarAutoCollapse) {
                closeSidebar();
                QTimer::singleShot(250, this, [this, idx](){ showPage(idx); });
            } else {
                showPage(idx);
            }
        });
        drawerLay->addWidget(btn);
        m_navButtons.append(btn);
    }
    drawerLay->addStretch();

    QLabel *ver = new QLabel("v3.2.3");
    ver->setObjectName("versionLabel");
    ver->setAlignment(Qt::AlignCenter);
    ver->setContentsMargins(0,0,0,16);
    drawerLay->addWidget(ver);

    bodyLay->addWidget(m_drawer);

    // 内容区域
    m_stack->addWidget(createHomePage());
    m_stack->addWidget(createTimelinePage());
    m_stack->addWidget(createChatPage());
    m_stack->addWidget(createQuizPage());
    m_stack->addWidget(createKnowledgeQAPage());
    m_stack->addWidget(createKnowledgeQuizPage());
    m_stack->addWidget(createGamePage());
    m_stack->addWidget(createSettingsPage());
    bodyLay->addWidget(m_stack, 1);

    mainLay->addLayout(bodyLay);

    setCentralWidget(central);
}

/* 侧边栏动画 */
void MainWindow::toggleSidebar()
{
    if (m_drawerOpen) closeSidebar(); else openSidebar();
}

void MainWindow::openSidebar()
{
    if (m_drawerOpen) return;
    m_drawerOpen = true;

    QPropertyAnimation *anim = new QPropertyAnimation(m_drawer, "maximumWidth", this);
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->setDuration(350);
    anim->setStartValue(0);
    anim->setEndValue(260);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    m_menuBtn->setText("◀");
}

void MainWindow::closeSidebar()
{
    if (!m_drawerOpen) return;
    m_drawerOpen = false;

    QPropertyAnimation *anim = new QPropertyAnimation(m_drawer, "maximumWidth", this);
    anim->setEasingCurve(QEasingCurve::InBack);
    anim->setDuration(300);
    anim->setStartValue(260);
    anim->setEndValue(0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    m_menuBtn->setText("☰");
}

void MainWindow::setActiveNav(int index)
{
    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setProperty("active", i == index);
        m_navButtons[i]->style()->unpolish(m_navButtons[i]);
        m_navButtons[i]->style()->polish(m_navButtons[i]);
    }
}

/* ================================================================== */
/*  页面切换                                                          */
/* ================================================================== */
void MainWindow::showPage(int index)
{
    m_stack->setCurrentIndex(index);
    setActiveNav(index);
    QStringList titles = {"国防装备","发展历程","AI 问答","AI 答题","知识问答","知识答题","激光防御","软件设置"};
    if (index >= 0 && index < titles.size())
        m_topTitle->setText(titles[index]);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}

/* ================================================================== */
/*  首页 — 装备卡片                                              */
/* ================================================================== */
QWidget* MainWindow::createHomePage()
{
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);

    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(28,20,28,20);
    lay->setSpacing(0);

    QLabel *hero = new QLabel("中国  国防装备");
    hero->setObjectName("heroTitle");
    hero->setAlignment(Qt::AlignCenter);
    lay->addWidget(hero);

    QLabel *sub = new QLabel("中国自主研发 · 守护和平");
    sub->setObjectName("subLabel");
    sub->setAlignment(Qt::AlignCenter);
    lay->addWidget(sub);
    lay->addSpacing(24);

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(16);

    QMap<QString,QString> catColor = {
        {"空军","#4488ff"},{"海军","#00aacc"},{"火箭军","#ff6633"},
        {"陆军","#66aa33"},{"空军/陆军","#4488ff"}
    };

    for (int i = 0; i < m_equipmentList.size(); ++i) {
        const auto &eq = m_equipmentList[i];
        QFrame *card = new QFrame();
        card->setObjectName("equipCard");
        card->setCursor(Qt::PointingHandCursor);
        card->installEventFilter(this);
        card->setProperty("equipIndex", i);

        // Initial state for entrance animation
        QGraphicsOpacityEffect *cardOpacity = new QGraphicsOpacityEffect(card);
        cardOpacity->setOpacity(0.0);
        card->setGraphicsEffect(cardOpacity);

        QVBoxLayout *cl = new QVBoxLayout(card); cl->setSpacing(4);
        QLabel *badge = new QLabel(eq.category);
        badge->setObjectName("catBadge");
        badge->setMaximumWidth(100);
        // 分类颜色是内容特定的（不是主题依赖的），只使用内联颜色
        badge->setStyleSheet(QString("color:%1;").arg(catColor.value(eq.category, "#c41e3a")));
        cl->addWidget(badge);
        QLabel *name = new QLabel(eq.name);
        name->setObjectName("equipName");
        name->setWordWrap(true); cl->addWidget(name);
        QLabel *desc = new QLabel(eq.shortDesc);
        desc->setObjectName("equipDesc");
        desc->setWordWrap(true); cl->addWidget(desc);
        QLabel *link = new QLabel("查看详情 →");
        link->setObjectName("equipLink");
        // 链接使用分类颜色
        link->setStyleSheet(QString("color:%1;").arg(catColor.value(eq.category, "#c41e3a")));
        cl->addWidget(link);

        grid->addWidget(card, i/3, i%3);

        // Staggered entrance animation
        QPropertyAnimation *cardAnim = new QPropertyAnimation(cardOpacity, "opacity", this);
        cardAnim->setDuration(300);
        cardAnim->setEasingCurve(QEasingCurve::OutCubic);
        cardAnim->setStartValue(0.0);
        cardAnim->setEndValue(1.0);
        QTimer::singleShot(100 + i * 60, cardAnim, [cardAnim]() {
            cardAnim->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }
    lay->addLayout(grid);
    lay->addStretch();
    scroll->setWidget(page);
    return scroll;
}

void MainWindow::showEquipmentDetail(const EquipmentInfo &eq)
{
    const Theme &t = m_darkMode ? DARK : LIGHT;
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(eq.name);
    dlg->setMinimumSize(560, 480);

    QVBoxLayout *lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(28,24,28,24);
    lay->setSpacing(10);

    QLabel *title = new QLabel(eq.name);
    title->setObjectName("heroTitle");
    title->setWordWrap(true); lay->addWidget(title);

    QLabel *cat = new QLabel("分类: " + eq.category);
    cat->setObjectName("dimLabel");
    lay->addWidget(cat);

    QFrame *sep = new QFrame(); sep->setFixedHeight(1);
    sep->setObjectName("separator");
    lay->addWidget(sep);

    QLabel *desc = new QLabel(eq.fullDesc);
    desc->setWordWrap(true);
    lay->addWidget(desc);

    QLabel *specTitle = new QLabel("📋 主要参数");
    specTitle->setObjectName("sectionTitle");
    lay->addWidget(specTitle);

    QFrame *specFrame = new QFrame();
    specFrame->setObjectName("specFrame");
    QVBoxLayout *specLay = new QVBoxLayout(specFrame);
    for (const QString &line : eq.specs.split("\n")) {
        if (line.trimmed().isEmpty()) continue;
        int cp = line.indexOf(":"); if (cp<0) cp = line.indexOf("：");
        if (cp > 0) {
            QLabel *l = new QLabel(QString("<b style='color:%1;'>%2</b>: %3")
                .arg(t.accent, line.left(cp).trimmed(), line.mid(cp+1).trimmed()));
            l->setWordWrap(true);
            l->setObjectName("dimLabel");
            specLay->addWidget(l);
        }
    }
    lay->addWidget(specFrame);
    lay->addStretch();

    QPushButton *closeBtn = new QPushButton("关  闭");
    closeBtn->setFixedWidth(100);
    closeBtn->setObjectName("primaryBtn");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    lay->addWidget(closeBtn, 0, Qt::AlignCenter);

    dlg->exec();
    dlg->deleteLater();
}

/* ================================================================== */
/*  Timeline                                                           */
/* ================================================================== */
/*  时间轴 — 数轴样式，支持拖动和缩放                     */
/* ================================================================== */
QWidget* MainWindow::createTimelinePage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(28,20,28,20);
    lay->setSpacing(0);

    QLabel *title = new QLabel(QString::fromUtf8("国防发展历程"));
    title->setObjectName("heroTitle");
    title->setAlignment(Qt::AlignCenter);
    lay->addWidget(title);

    QLabel *sub = new QLabel(QString::fromUtf8("回顾过去 · 立足现在 · 展望未来"));
    sub->setObjectName("subLabel");
    sub->setAlignment(Qt::AlignCenter);
    lay->addWidget(sub);
    lay->addSpacing(10);

    m_timelineWidget = new TimelineWidget();

    QList<TimelineWidget::Item> items = {
        {QString::fromUtf8("过去"), QString::fromUtf8("1927年"),
         QString::fromUtf8("南昌起义"),
         QString::fromUtf8("中国共产党独立领导武装斗争的开始")},
        {QString::fromUtf8("过去"), QString::fromUtf8("1949年"),
         QString::fromUtf8("新中国成立"),
         QString::fromUtf8("国防建设进入新阶段")},
        {QString::fromUtf8("过去"), QString::fromUtf8("1950-1953年"),
         QString::fromUtf8("抗美援朝"),
         QString::fromUtf8("保家卫国，展现国威军威")},
        {QString::fromUtf8("现在"), QString::fromUtf8("2012年"),
         QString::fromUtf8("辽宁舰入列"),
         QString::fromUtf8("首艘航母正式交付海军")},
        {QString::fromUtf8("现在"), QString::fromUtf8("2017年"),
         QString::fromUtf8("歼-20服役"),
         QString::fromUtf8("第五代战斗机正式列装")},
        {QString::fromUtf8("现在"), QString::fromUtf8("2019年"),
         QString::fromUtf8("东风-41亮相"),
         QString::fromUtf8("洲际弹道导弹首次公开")},
        {QString::fromUtf8("现在"), QString::fromUtf8("2022年"),
         QString::fromUtf8("福建舰下水"),
         QString::fromUtf8("首艘电磁弹射型航母下水")},
        {QString::fromUtf8("未来"), QString::fromUtf8("2035年"),
         QString::fromUtf8("现代化军队"),
         QString::fromUtf8("基本实现国防和军队现代化")},
        {QString::fromUtf8("未来"), QString::fromUtf8("2049年"),
         QString::fromUtf8("世界一流军队"),
         QString::fromUtf8("全面建成世界一流军队")}
    };
    m_timelineWidget->setItems(items);
    m_timelineWidget->setDarkMode(m_darkMode);

    lay->addWidget(m_timelineWidget, 1);

    return page;
}

/* ================================================================== */
/*  聊天                                                                */
/* ================================================================== */
QWidget* MainWindow::createChatPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(20,16,20,16);
    lay->setSpacing(8);

    QLabel *title = new QLabel("🤖  AI 国防知识助手");
    title->setObjectName("sectionTitle");
    title->setAlignment(Qt::AlignCenter); lay->addWidget(title);

    // 状态栏
    QFrame *sBar = new QFrame();
    sBar->setObjectName("statusBar");
    QHBoxLayout *sl = new QHBoxLayout(sBar); sl->setContentsMargins(0,0,0,0);
    m_statusDot->setFixedSize(8,8);
    m_statusDot->setObjectName("statusDot");
    m_statusDot->setProperty("status", "disconnected");
    sl->addWidget(m_statusDot);
    m_statusLabel->setText("未连接");
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setProperty("status", "disconnected");
    sl->addWidget(m_statusLabel); sl->addStretch();
    QPushButton *ck = new QPushButton("检查连接");
    ck->setObjectName("primaryBtn");
    ck->setFixedHeight(28);
    connect(ck, &QPushButton::clicked, this, &MainWindow::checkAiService);
    sl->addWidget(ck);
    lay->addWidget(sBar);

    m_chatDisplay->setObjectName("chatDisplay");
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setHtml(
        "<p style='color:#888;text-align:center;margin:40px 0;'>"
        "<b style='color:#ffd700;'>欢迎来到国防知识 AI 助手</b><br>"
        "我可以帮您解答国防历史、军事科技等方面的问题</p>");
    lay->addWidget(m_chatDisplay, 1);

    QFrame *inputF = new QFrame();
    QHBoxLayout *il = new QHBoxLayout(inputF); il->setContentsMargins(0,0,0,0); il->setSpacing(8);
    m_chatInput->setObjectName("chatInput");
    m_chatInput->setPlaceholderText("请输入问题，Enter 发送...");
    m_chatInput->setMaximumHeight(56);
    m_chatInput->installEventFilter(this);
    il->addWidget(m_chatInput, 1);
    QPushButton *sendBtn = new QPushButton("发送");
    sendBtn->setFixedSize(72,40);
    sendBtn->setObjectName("primaryBtn");
    connect(sendBtn, &QPushButton::clicked, this, &MainWindow::sendChatMessage);
    il->addWidget(sendBtn);
    lay->addWidget(inputF);

    QWidget *quickW = new QWidget();
    QHBoxLayout *ql = new QHBoxLayout(quickW); ql->setContentsMargins(0,0,0,0); ql->setSpacing(6);
    for (const auto &q : {"什么是国防？","中国国防政策","介绍歼-20"}) {
        QPushButton *b = new QPushButton(q);
        b->setObjectName("quickBtn");
        connect(b, &QPushButton::clicked, this, [this,q](){ m_chatInput->setText(q); });
        ql->addWidget(b);
    }
    ql->addStretch();
    lay->addWidget(quickW);
    return page;
}

/* ================================================================== */
/*  答题                                                                */
/* ================================================================== */
QWidget* MainWindow::createQuizPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(20,16,20,16);
    lay->setSpacing(10);

    QLabel *title = new QLabel("📝  AI 国防知识答题");
    title->setObjectName("sectionTitle");
    title->setAlignment(Qt::AlignCenter); lay->addWidget(title);

    QLabel *desc = new QLabel("AI 自动出题，你来作答！");
    desc->setObjectName("dimLabel");
    desc->setAlignment(Qt::AlignCenter); lay->addWidget(desc);

    QFrame *cfg = new QFrame();
    cfg->setObjectName("quizCfg");
    QHBoxLayout *cl = new QHBoxLayout(cfg); cl->setContentsMargins(0,0,0,0);
    cl->addWidget(new QLabel("题量:"));
    m_quizDifficulty->addItems({"3题","5题","10题"});
    cl->addWidget(m_quizDifficulty);
    cl->addStretch();
    m_quizScoreLabel->setObjectName("quizScoreLabel");
    m_quizScoreLabel->setText("得分: -");
    cl->addWidget(m_quizScoreLabel);
    QPushButton *gen = new QPushButton("生成题目");
    gen->setObjectName("primaryBtn");
    gen->setFixedHeight(28);
    connect(gen, &QPushButton::clicked, this, &MainWindow::generateQuiz);
    cl->addWidget(gen);
    lay->addWidget(cfg);

    m_quizProgress->setObjectName("quizProgress");
    m_quizProgress->setText("点击「生成题目」开始");
    m_quizProgress->setAlignment(Qt::AlignCenter);
    lay->addWidget(m_quizProgress);

    m_quizQuestion->setObjectName("quizQuestion");
    m_quizQuestion->setReadOnly(true);
    m_quizQuestion->setHtml("<p style='color:#888;text-align:center;'>等待生成题目...</p>");
    m_quizQuestion->setMinimumHeight(100);
    lay->addWidget(m_quizQuestion);

    QFrame *ansF = new QFrame();
    QVBoxLayout *ansL = new QVBoxLayout(ansF); ansL->setSpacing(6); ansL->setContentsMargins(0,0,0,0);
    for (int i = 0; i < 4; ++i) {
        QPushButton *btn = new QPushButton();
        btn->setMinimumHeight(40);
        btn->setObjectName("ansBtn");
        btn->setVisible(false);
        int idx = i;
        connect(btn, &QPushButton::clicked, this, [this,idx](){ selectAnswer(idx); });
        ansL->addWidget(btn);
        m_quizButtons.append(btn);
    }
    lay->addWidget(ansF);

    m_quizFeedback->setObjectName("feedbackLabel");
    m_quizFeedback->setAlignment(Qt::AlignCenter);
    m_quizFeedback->hide();
    lay->addWidget(m_quizFeedback);

    m_quizNextBtn = new QPushButton("下一题 →");
    m_quizNextBtn->setObjectName("quizNextBtn");
    m_quizNextBtn->hide();
    connect(m_quizNextBtn, &QPushButton::clicked, this, &MainWindow::nextQuestion);
    lay->addWidget(m_quizNextBtn, 0, Qt::AlignCenter);

    lay->addStretch();
    return page;
}

/* ---- 答题逻辑 ---- */
void MainWindow::generateQuiz()
{
    // 防止重复生成：如果已经在接收响应则不允许重新生成
    if (m_quizReceiving) return;

    int count = 3;
    QString txt = m_quizDifficulty->currentText();
    if (txt.contains("5")) count = 5; else if (txt.contains("10")) count = 10;
    m_quizTotal = count; m_quizCurrent = 0; m_quizCorrect = 0;
    m_quizQuestions.clear(); m_quizRawResponse.clear();
    m_quizReceiving = true; m_quizSelectedAnswer = -1;

    m_quizProgress->setText("正在出题...");
    m_quizQuestion->setHtml("<p style='color:#888;text-align:center;'>AI 正在出题...</p>");
    m_quizFeedback->hide();
    for (auto b : m_quizButtons) b->setVisible(false);
    m_quizNextBtn->hide();

    QString provider = m_settingsManager->aiProvider();
    QString model = provider == "ollama" ? m_settingsManager->ollamaModel() : m_settingsManager->deepseekModel();

    QString prompt = QString(
        "你是国防知识出题专家。生成%1道关于中国国防的单项选择题。\n"
        "严格按以下JSON格式输出，不要输出其他内容：\n"
        "[{\"question\":\"题目\",\"options\":[\"A选项\",\"B选项\",\"C选项\",\"D选项\"],"
        "\"answer\":正确答案索引0到3,\"explanation\":\"解析\"}]\n"
        "只输出JSON数组，不要加```json等标记。").arg(count);

    // 断开所有旧连接（包括聊天模式的 onErrorOccurred）
    disconnect(m_aiServiceManager, &AiServiceManager::responseChunk, this, &MainWindow::onQuizResponseChunk);
    disconnect(m_aiServiceManager, &AiServiceManager::responseReceived, this, &MainWindow::onQuizResponseReceived);
    disconnect(m_aiServiceManager, &AiServiceManager::errorOccurred, this, &MainWindow::onQuizError);
    disconnect(m_aiServiceManager, &AiServiceManager::responseChunk, this, &MainWindow::onResponseChunk);
    disconnect(m_aiServiceManager, &AiServiceManager::responseReceived, this, &MainWindow::onResponseReceived);
    disconnect(m_aiServiceManager, &AiServiceManager::errorOccurred, this, &MainWindow::onErrorOccurred);

    connect(m_aiServiceManager, &AiServiceManager::responseChunk, this, &MainWindow::onQuizResponseChunk, Qt::UniqueConnection);
    connect(m_aiServiceManager, &AiServiceManager::responseReceived, this, &MainWindow::onQuizResponseReceived, Qt::UniqueConnection);
    connect(m_aiServiceManager, &AiServiceManager::errorOccurred, this, &MainWindow::onQuizError, Qt::UniqueConnection);

    // 根据AI服务商发送消息
    if (provider == "ollama") {
        m_aiServiceManager->sendMessage(prompt, model, "只输出JSON，不输出其他内容。", "ollama", m_settingsManager->ollamaUrl());
    } else {
        m_aiServiceManager->sendMessage(prompt, model, "只输出JSON，不输出其他内容。", "deepseek", m_settingsManager->deepseekApiUrl(), m_settingsManager->deepseekApiKey());
    }
}

void MainWindow::onQuizResponseChunk(const QString &c) { m_quizRawResponse += c; }

void MainWindow::onQuizResponseReceived(const QString &)
{
    m_quizReceiving = false;
    // 优化：只需断开答题模式信号，无需重新连接聊天模式
    disconnect(m_aiServiceManager, &AiServiceManager::responseChunk, this, &MainWindow::onQuizResponseChunk);
    disconnect(m_aiServiceManager, &AiServiceManager::responseReceived, this, &MainWindow::onQuizResponseReceived);
    disconnect(m_aiServiceManager, &AiServiceManager::errorOccurred, this, &MainWindow::onQuizError);

    QString jsonStr = m_quizRawResponse.trimmed();
    QRegularExpression re("```(?:json)?\\s*([\\s\\S]*?)```");
    QRegularExpressionMatch m = re.match(jsonStr);
    if (m.hasMatch()) jsonStr = m.captured(1).trimmed();

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (!doc.isArray()) {
        m_quizQuestion->setHtml("<p style='color:#ff4444;text-align:center;'>AI 输出格式异常，请重试</p>");
        return;
    }
    m_quizQuestions.clear();
    for (const QJsonValue &v : doc.array()) {
        QJsonObject obj = v.toObject();
        if (obj.contains("question") && obj.contains("options") && obj.contains("answer")) {
            QVariantMap vm;
            vm["question"] = obj.value("question").toString();
            vm["options"] = obj.value("options").toVariant().toStringList();
            vm["answer"] = obj.value("answer").toInt();
            vm["explanation"] = obj.value("explanation").toString();
            m_quizQuestions.append(vm);
        }
    }
    if (m_quizQuestions.isEmpty()) {
        m_quizQuestion->setHtml("<p style='color:#ff4444;text-align:center;'>未能解析出有效题目，请重试</p>");
        return;
    }
    m_quizTotal = m_quizQuestions.size(); m_quizCurrent = 0; m_quizCorrect = 0;
    m_quizScoreLabel->setText("得分: 0");
    showCurrentQuizQuestion();
}

void MainWindow::onQuizError(const QString &error)
{
    m_quizReceiving = false;
    // 优化：只需断开答题模式信号
    disconnect(m_aiServiceManager, &AiServiceManager::responseChunk, this, &MainWindow::onQuizResponseChunk);
    disconnect(m_aiServiceManager, &AiServiceManager::responseReceived, this, &MainWindow::onQuizResponseReceived);
    disconnect(m_aiServiceManager, &AiServiceManager::errorOccurred, this, &MainWindow::onQuizError);
    m_quizQuestion->setHtml(QString("<p style='color:#ff4444;text-align:center;'>%1</p>").arg(escapeHtml(error)));
}

void MainWindow::showCurrentQuizQuestion()
{
    if (m_quizCurrent >= m_quizQuestions.size()) { finishQuiz(); return; }
    const QVariantMap &q = m_quizQuestions[m_quizCurrent];
    m_quizProgress->setText(QString("第 %1 / %2 题").arg(m_quizCurrent+1).arg(m_quizTotal));
    m_quizQuestion->setHtml(QString("<p style='font-size:15px;'>%1</p>").arg(escapeHtml(q.value("question").toString())));
    QStringList opts = q.value("options").toStringList();
    QStringList labels = {"A","B","C","D"};
    for (int i = 0; i < 4; ++i) {
        if (i < opts.size()) {
            m_quizButtons[i]->setText(labels[i]+". "+opts[i]);
            m_quizButtons[i]->setVisible(true); m_quizButtons[i]->setEnabled(true);
            // 重置动态属性
            m_quizButtons[i]->setProperty("correct", QVariant());
            m_quizButtons[i]->setProperty("wrong", QVariant());
            m_quizButtons[i]->setProperty("dim", QVariant());
            m_quizButtons[i]->style()->unpolish(m_quizButtons[i]);
            m_quizButtons[i]->style()->polish(m_quizButtons[i]);
        } else m_quizButtons[i]->setVisible(false);
    }
    m_quizFeedback->hide();
    m_quizNextBtn->hide();
    m_quizSelectedAnswer = -1;
}

void MainWindow::selectAnswer(int idx)
{
    if (m_quizSelectedAnswer >= 0) return;
    m_quizSelectedAnswer = idx;
    const QVariantMap &q = m_quizQuestions[m_quizCurrent];
    int correct = q.value("answer").toInt();
    QString explanation = q.value("explanation").toString();

    for (int i = 0; i < 4; ++i) {
        m_quizButtons[i]->setEnabled(false);
        // 先清除所有动态属性
        m_quizButtons[i]->setProperty("correct", QVariant());
        m_quizButtons[i]->setProperty("wrong", QVariant());
        m_quizButtons[i]->setProperty("dim", QVariant());

        if (i == correct)
            m_quizButtons[i]->setProperty("correct", true);
        else if (i == idx && idx != correct)
            m_quizButtons[i]->setProperty("wrong", true);
        else
            m_quizButtons[i]->setProperty("dim", true);

        // 强制重新评估样式
        m_quizButtons[i]->style()->unpolish(m_quizButtons[i]);
        m_quizButtons[i]->style()->polish(m_quizButtons[i]);
    }

    bool ok = (idx == correct);
    if (ok) {
        m_quizCorrect++;
        m_quizFeedback->setStyleSheet("color:#00cc66;");
        m_quizFeedback->setText("✓ 正确！");
    } else {
        m_quizFeedback->setStyleSheet("color:#ff4444;");
        m_quizFeedback->setText("✗ 错误");
    }
    m_quizFeedback->show();
    if (!explanation.isEmpty()) m_quizFeedback->setText(m_quizFeedback->text() + "  " + explanation);
    m_quizScoreLabel->setText(QString("得分: %1/%2").arg(m_quizCorrect).arg(m_quizCurrent+1));

    m_quizNextBtn->setText(m_quizCurrent+1 < m_quizQuestions.size() ? "下一题 →" : "查看结果");
    m_quizNextBtn->show();
}

/* ---- Markdown rendering ---- */
QString MainWindow::renderMarkdown(const QString &md) const
{
    QString html;
    QStringList lines = md.split('\n');
    bool inCodeBlock = false;
    QString codeContent;
    int listType = 0; // 0=none, 1=ul, 2=ol

    auto closeList = [&]() {
        if (listType == 1) { html += "</ul>"; listType = 0; }
        else if (listType == 2) { html += "</ol>"; listType = 0; }
    };

    for (int i = 0; i < lines.size(); ++i) {
        QString raw = lines[i];

        if (raw.trimmed().startsWith("```")) {
            if (inCodeBlock) {
                html += "<pre><code>" + codeContent.toHtmlEscaped() + "</code></pre>";
                codeContent.clear();
                inCodeBlock = false;
            } else {
                closeList();
                inCodeBlock = true;
            }
            continue;
        }
        if (inCodeBlock) {
            codeContent += raw + "\n";
            continue;
        }

        // Empty line → close any open list
        if (raw.trimmed().isEmpty()) {
            closeList();
            continue;
        }

        // Horizontal rule --- / *** / ___
        {
            QRegularExpression hrRe("^(-{3,}|\\*{3,}|_{3,})\\s*$");
            if (hrRe.match(raw.trimmed()).hasMatch()) {
                closeList();
                html += "<hr style='border:none;border-top:2px solid #555;margin:12px 0;'>";
                continue;
            }
        }

        // Blockquote > text
        if (raw.startsWith("> ")) {
            closeList();
            html += "<blockquote style='border-left:4px solid #888;margin:8px 0;padding:4px 14px;color:#888;'>"
                    + processInline(raw.mid(2)) + "</blockquote>";
            continue;
        }

        // Headings
        if (raw.startsWith("### "))     { closeList(); html += "<h3>" + processInline(raw.mid(4)) + "</h3>"; continue; }
        if (raw.startsWith("## "))      { closeList(); html += "<h2>" + processInline(raw.mid(3)) + "</h2>"; continue; }
        if (raw.startsWith("# "))       { closeList(); html += "<h1>" + processInline(raw.mid(2)) + "</h1>"; continue; }

        // Unordered list
        QRegularExpression ulRe("^[-*+]\\s+(.+)$");
        auto ulM = ulRe.match(raw);
        if (ulM.hasMatch()) {
            if (listType != 1) { closeList(); html += "<ul>"; listType = 1; }
            html += "<li>" + processInline(ulM.captured(1)) + "</li>";
            continue;
        }

        // Ordered list – 手动编号避免 QTextEdit 的 <ol> 渲染缺陷
        QRegularExpression olRe("^(\\d+)\\.\\s+(.+)$");
        auto olM = olRe.match(raw);
        if (olM.hasMatch()) {
            closeList();
            html += QString("<p style='margin:4px 0;padding-left:1em;'><b>%1.</b> %2</p>")
                        .arg(olM.captured(1), processInline(olM.captured(2)));
            continue;
        }

        closeList();
        html += "<p>" + processInline(raw) + "</p>";
    }

    closeList();
    if (inCodeBlock) {
        html += "<pre><code>" + codeContent.toHtmlEscaped() + "</code></pre>";
    }
    return html;
}

QString MainWindow::processInline(const QString &text) const
{
    QString r = text;

    // Step 1: protect inline code with opaque placeholders
    QRegularExpression codeRe("`([^`]+)`");
    QStringList codes;
    int pos = 0;
    QString result;
    QRegularExpressionMatchIterator it = codeRe.globalMatch(r);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        result += r.mid(pos, m.capturedStart() - pos);
        codes.append(m.captured(1));
        result += QStringLiteral("\x01CODE%1\x01").arg(codes.size() - 1);
        pos = m.capturedEnd();
    }
    result += r.mid(pos);
    r = result;

    // Step 2: escape HTML
    r = r.toHtmlEscaped();

    // Step 3: apply inline formatting and links
    r.replace(QRegularExpression("\\*\\*\\*(.+?)\\*\\*\\*"), "<b><i>\\1</i></b>");
    r.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>");
    r.replace(QRegularExpression("(?<!\\*)\\*(?!\\*)(.+?)(?<!\\*)\\*(?!\\*)"), "<i>\\1</i>");
    r.replace(QRegularExpression("~~(.+?)~~"), "<s>\\1</s>");
    r.replace(QRegularExpression("\\[([^\\]]+)\\]\\(([^)]+)\\)"),
              "<a href='\\2' target='_blank'>\\1</a>");
    r.replace(QRegularExpression("!\\[([^\\]]*)\\]\\(([^)]+)\\)"),
              "<a href='\\2' target='_blank'>[图片] \\1</a>");

    // Step 4: restore inline code (content gets HTML-escaped here)
    for (int i = 0; i < codes.size(); ++i) {
        r.replace(QStringLiteral("\x01CODE%1\x01").arg(i),
                  "<code>" + codes[i].toHtmlEscaped() + "</code>");
    }

    r.replace("\n", "<br>");
    return r;
}

void MainWindow::nextQuestion() { m_quizCurrent++; showCurrentQuizQuestion(); }

void MainWindow::finishQuiz()
{
    const Theme &t = m_darkMode ? DARK : LIGHT;
    int pct = m_quizTotal>0 ? m_quizCorrect*100/m_quizTotal : 0;
    QString grade = pct>=90?"🏆 优秀！":pct>=70?"👍 良好":pct>=50?"📖 还需努力":"💪 再接再厉";
    m_quizQuestion->setHtml(QString(
        "<div style='text-align:center;margin:20px;'>"
        "<p style='font-size:36px;font-weight:bold;color:%1;'>%2 / %3</p>"
        "<p style='font-size:16px;color:%4;'>%5</p></div>")
        .arg(t.accent).arg(m_quizCorrect).arg(m_quizTotal).arg(t.text).arg(grade));
    m_quizProgress->setText("答题完成！");
    m_quizScoreLabel->setText(QString("最终: %1/%2").arg(m_quizCorrect).arg(m_quizTotal));
    for (auto b : m_quizButtons) b->setVisible(false);
    m_quizFeedback->hide();
    m_quizNextBtn->setText("再来一轮");
    m_quizNextBtn->show();
    disconnect(m_quizNextBtn, &QPushButton::clicked, this, &MainWindow::nextQuestion);
    connect(m_quizNextBtn, &QPushButton::clicked, this, &MainWindow::generateQuiz);
}

/* ================================================================== */
/*  游戏 — 激光炮塔防御                                              */
/* ================================================================== */
QWidget* MainWindow::createGamePage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    m_gameWidget = new GameWidget();
    m_gameWidget->setDarkMode(m_darkMode);
    lay->addWidget(m_gameWidget, 1);

    QFrame *gameBar = new QFrame();
    gameBar->setFixedHeight(40);
    gameBar->setObjectName("gameBar");
    QHBoxLayout *gbLay = new QHBoxLayout(gameBar);
    gbLay->setContentsMargins(12,0,12,0);

    QPushButton *gameStartBtn = new QPushButton("开始游戏");
    gameStartBtn->setObjectName("gameStartBtn");
    gameStartBtn->setFixedSize(100,28);
    gameStartBtn->setStyleSheet(
        "QPushButton { background:#c41e3a; color:white; border:none; border-radius:4px; font-size:13px; }"
        "QPushButton:hover { background:#e8294a; }");

    QLabel *gameStatus = new QLabel("点击「开始游戏」开始");
    gameStatus->setObjectName("dimLabel");
    gameStatus->setStyleSheet("color:#888; font-size:12px;");

    gbLay->addWidget(gameStartBtn);
    gbLay->addSpacing(8);
    gbLay->addWidget(gameStatus);
    gbLay->addStretch();
    lay->addWidget(gameBar);

    connect(gameStartBtn, &QPushButton::clicked, this, [this, gameStartBtn, gameStatus]() {
        if (m_gameWidget->isPaused() && !m_gameWidget->isGameOver()) {
            m_gameWidget->resumeGame();
            gameStartBtn->setText("暂停");
            gameStatus->setText("游戏进行中");
        } else if (!m_gameWidget->isPaused() && !m_gameWidget->isGameOver()) {
            m_gameWidget->pauseGame();
            gameStartBtn->setText("继续");
            gameStatus->setText("已暂停");
        } else {
            m_gameWidget->startGame();
            gameStartBtn->setText("暂停");
            gameStatus->setText("游戏进行中");
        }
    });

    return page;
}

/* ================================================================== */
/*  设置                                                                */
/* ================================================================== */
QWidget* MainWindow::createSettingsPage()
{
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);

    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(36,24,36,24);

    QLabel *title = new QLabel("软件设置");
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter); lay->addWidget(title);
    QLabel *sub = new QLabel("自定义您的使用体验");
    sub->setObjectName("subLabel");
    sub->setAlignment(Qt::AlignCenter); lay->addWidget(sub);
    lay->addSpacing(20);

    // 外观设置区域
    QFrame *af = new QFrame();
    af->setObjectName("sectionFrame");
    QVBoxLayout *al = new QVBoxLayout(af); al->setSpacing(10);
    QLabel *appTitle = new QLabel("🎨  个性化");
    appTitle->setObjectName("sectionTitle");
    al->addWidget(appTitle);
    al->addWidget(new QLabel("主题:"));
    QHBoxLayout *tl = new QHBoxLayout();
    m_autoBtn->setText(QString::fromUtf8("跟随系统")); m_autoBtn->setCheckable(true); m_autoBtn->setMinimumSize(100,36);
    m_autoBtn->setObjectName("themeToggleBtn");
    m_darkBtn->setText("深色"); m_darkBtn->setCheckable(true); m_darkBtn->setMinimumSize(70,36);
    m_darkBtn->setObjectName("themeToggleBtn");
    m_lightBtn->setText("浅色"); m_lightBtn->setCheckable(true); m_lightBtn->setMinimumSize(70,36);
    m_lightBtn->setObjectName("themeToggleBtn");
    connect(m_autoBtn, &QPushButton::clicked, this, [this](){
        m_themeMode = "system";
        m_darkMode = systemDarkMode();
        m_autoBtn->setChecked(true); m_darkBtn->setChecked(false); m_lightBtn->setChecked(false);
        applyTheme(); saveSettings();
    });
    connect(m_darkBtn, &QPushButton::clicked, this, [this](){
        m_themeMode="dark"; m_darkMode=true;
        m_autoBtn->setChecked(false); m_darkBtn->setChecked(true); m_lightBtn->setChecked(false);
        applyTheme(); saveSettings();
    });
    connect(m_lightBtn, &QPushButton::clicked, this, [this](){
        m_themeMode="light"; m_darkMode=false;
        m_autoBtn->setChecked(false); m_darkBtn->setChecked(false); m_lightBtn->setChecked(true);
        applyTheme(); saveSettings();
    });
    tl->addWidget(m_autoBtn); tl->addWidget(m_darkBtn); tl->addWidget(m_lightBtn); tl->addStretch();
    al->addLayout(tl);

    // 亚克力效果开关
    QHBoxLayout *acrylicLay = new QHBoxLayout();
    QLabel *acrylicLabel = new QLabel("亚克力效果:");
    acrylicLay->addWidget(acrylicLabel);
    QCheckBox *acrylicCheck = new QCheckBox("启用 Win11 亚克力风格");
    acrylicCheck->setObjectName("acrylicCheck");
    acrylicCheck->setChecked(m_acrylicMode);
    connect(acrylicCheck, &QCheckBox::toggled, this, [this](bool checked){
        m_acrylicMode = checked;
        applyTheme(); saveSettings();
    });
    acrylicLay->addWidget(acrylicCheck);
    acrylicLay->addStretch();
    al->addLayout(acrylicLay);

    // 侧边栏自动收回开关
    QHBoxLayout *sidebarLay = new QHBoxLayout();
    QLabel *sidebarLabel = new QLabel("侧边栏:");
    sidebarLay->addWidget(sidebarLabel);
    QCheckBox *sidebarAutoCheck = new QCheckBox("导航后自动收回侧边栏");
    sidebarAutoCheck->setObjectName("sidebarAutoCheck");
    sidebarAutoCheck->setChecked(m_sidebarAutoCollapse);
    connect(sidebarAutoCheck, &QCheckBox::toggled, this, [this](bool checked){
        m_sidebarAutoCollapse = checked;
        saveSettings();
    });
    sidebarLay->addWidget(sidebarAutoCheck);
    sidebarLay->addStretch();
    al->addLayout(sidebarLay);

    lay->addWidget(af);

    // AI 服务配置区域
    QFrame *aiFrame = new QFrame();
    aiFrame->setObjectName("sectionFrame");
    QVBoxLayout *aiLayout = new QVBoxLayout(aiFrame);
    aiLayout->setSpacing(10);

    QLabel *aiTitle = new QLabel("🤖  AI 服务设置");
    aiTitle->setObjectName("sectionTitle");
    aiLayout->addWidget(aiTitle);

    // AI服务商选项卡
    aiLayout->addWidget(new QLabel("AI 服务商:"));
    m_aiProviderCombo = new QComboBox();
    m_aiProviderCombo->addItem("Ollama（本地）");
    m_aiProviderCombo->addItem("DeepSeek（云端）");
    aiLayout->addWidget(m_aiProviderCombo);
    connect(m_aiProviderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        m_aiSettingsStack->setCurrentIndex(index);
        if (index == 0) {
            m_settingsManager->setAiProvider("ollama");
            // 恢复Ollama已检测模型
            QStringList models = m_settingsManager->discoveredOllamaModels();
            onModelsFetched(models);
        } else {
            m_settingsManager->setAiProvider("deepseek");
            // 恢复DeepSeek已检测模型
            QStringList models = m_settingsManager->discoveredDeepseekModels();
            onModelsFetched(models);
        }
    });

    // AI设置堆叠窗口
    m_aiSettingsStack = new QStackedWidget();

    // Ollama设置页
    QWidget *ollamaPage = new QWidget();
    QVBoxLayout *ollamaLayout = new QVBoxLayout(ollamaPage);
    ollamaLayout->setContentsMargins(0, 0, 0, 0);
    ollamaLayout->setSpacing(8);
    ollamaLayout->addWidget(new QLabel("服务地址:"));
    m_urlInput->setText("http://localhost:11434");
    connect(m_urlInput, &QLineEdit::textChanged, this, &MainWindow::debouncedSaveSettings);
    ollamaLayout->addWidget(m_urlInput);
    ollamaLayout->addWidget(new QLabel("模型名称:"));
    m_modelSelectCombo->setEditable(true);
    m_modelSelectCombo->lineEdit()->setPlaceholderText("输入或选择模型名称");
    m_modelSelectCombo->addItem("自定义");
    connect(m_modelSelectCombo, &QComboBox::currentTextChanged, this, &MainWindow::debouncedSaveSettings);
    ollamaLayout->addWidget(m_modelSelectCombo);
    QLabel *modelHint = new QLabel("提示: 选择「自定义」时可手动输入模型名称");
    modelHint->setObjectName("dimLabel");
    modelHint->setWordWrap(true);
    ollamaLayout->addWidget(modelHint);
    QLabel *ollamaHint = new QLabel("提示: Ollama无需填写API密钥，请确保Ollama服务已启动");
    ollamaHint->setObjectName("dimLabel");
    ollamaHint->setWordWrap(true);
    ollamaLayout->addWidget(ollamaHint);
    ollamaLayout->addStretch();
    m_aiSettingsStack->addWidget(ollamaPage);

    // DeepSeek设置页
    QWidget *deepseekPage = new QWidget();
    QVBoxLayout *deepseekLayout = new QVBoxLayout(deepseekPage);
    deepseekLayout->setContentsMargins(0, 0, 0, 0);
    deepseekLayout->setSpacing(8);
    deepseekLayout->addWidget(new QLabel("API 地址:"));
    m_deepseekApiUrlInput = new QLineEdit();
    m_deepseekApiUrlInput->setText("https://api.deepseek.com/v1");
    connect(m_deepseekApiUrlInput, &QLineEdit::textChanged, this, &MainWindow::debouncedSaveSettings);
    deepseekLayout->addWidget(m_deepseekApiUrlInput);
    deepseekLayout->addWidget(new QLabel("API 密钥:"));
    m_deepseekApiKeyInput = new QLineEdit();
    m_deepseekApiKeyInput->setEchoMode(QLineEdit::Password);
    m_deepseekApiKeyInput->setPlaceholderText("请输入DeepSeek API密钥");
    connect(m_deepseekApiKeyInput, &QLineEdit::textChanged, this, &MainWindow::debouncedSaveSettings);
    deepseekLayout->addWidget(m_deepseekApiKeyInput);
    deepseekLayout->addWidget(new QLabel("模型名称:"));
    QComboBox *deepseekModelCombo = new QComboBox();
    deepseekModelCombo->setObjectName("deepseekModelCombo");
    deepseekModelCombo->setEditable(false);
    deepseekModelCombo->addItem("deepseek-chat");
    deepseekModelCombo->addItem("deepseek-coder");
    deepseekModelCombo->addItem("自定义");
    connect(deepseekModelCombo, &QComboBox::currentTextChanged, this, &MainWindow::debouncedSaveSettings);
    deepseekLayout->addWidget(deepseekModelCombo);
    QLabel *deepseekHint = new QLabel("提示: 请在DeepSeek官网获取API密钥: https://platform.deepseek.com");
    deepseekHint->setObjectName("dimLabel");
    deepseekHint->setWordWrap(true);
    deepseekLayout->addWidget(deepseekHint);
    deepseekLayout->addStretch();
    m_aiSettingsStack->addWidget(deepseekPage);

    aiLayout->addWidget(m_aiSettingsStack);

    // 系统提示词（两种服务商共用）
    aiLayout->addWidget(new QLabel("系统提示词:"));
    m_promptInput->setObjectName("promptInput");
    m_promptInput->setMaximumHeight(70);
    connect(m_promptInput, &QTextEdit::textChanged, this, &MainWindow::debouncedSaveSettings);
    aiLayout->addWidget(m_promptInput);

    lay->addWidget(aiFrame);

    QPushButton *resetBtn = new QPushButton("恢复默认设置");
    resetBtn->setObjectName("primaryBtn");
    resetBtn->setFixedHeight(36);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::resetSettings);
    lay->addWidget(resetBtn, 0, Qt::AlignCenter);

    QPushButton *changelogBtn = new QPushButton("查看更新日志");
    changelogBtn->setObjectName("resetBtn");
    changelogBtn->setFixedHeight(48);  // 增大按钮
    changelogBtn->setMinimumWidth(200);
    changelogBtn->setStyleSheet("font-size: 15px;");
    connect(changelogBtn, &QPushButton::clicked, this, &MainWindow::showChangelog);
    lay->addWidget(changelogBtn, 0, Qt::AlignCenter);

    lay->addStretch();
    scroll->setWidget(page);
    return scroll;
}

/* ================================================================== */
/*  事件过滤器                                                        */
/* ================================================================== */
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *w = qobject_cast<QWidget*>(watched);
        if (w) {
            bool ok; int idx = w->property("equipIndex").toInt(&ok);
            if (ok && idx >= 0 && idx < m_equipmentList.size()) {
                showEquipmentDetail(m_equipmentList[idx]);
                return true;
            }
            // 知识问答页面卡片点击 - 切换答案可见性
            if (w->property("qaCard").toBool()) {
                int qaIdx = w->property("qaIdx").toInt();
                QLabel *ae = w->findChild<QLabel*>(QString("qaAnswer%1").arg(qaIdx));
                if (ae) {
                    ae->setVisible(!ae->isVisible());
                    return true;
                }
            }
        }
    }
    if (watched == m_chatInput && event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Return && !(ke->modifiers() & Qt::ShiftModifier)) {
            sendChatMessage(); return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

/* ================================================================== */
/*  聊天逻辑                                                         */
/* ================================================================== */
void MainWindow::checkAiService()
{
    QString provider = m_settingsManager->aiProvider();
    if (provider == "ollama") {
        m_aiServiceManager->checkService("ollama", m_settingsManager->ollamaUrl());
    } else {
        m_aiServiceManager->checkService("deepseek", m_settingsManager->deepseekApiUrl(), m_settingsManager->deepseekApiKey());
    }
}

/* ================================================================== */
/*  启动时静默模型检测                                                  */
/* ================================================================== */
void MainWindow::startupModelDetection()
{
    QString ollamaUrl = m_settingsManager->ollamaUrl();
    QString deepseekUrl = m_settingsManager->deepseekApiUrl();
    QString deepseekApiKey = m_settingsManager->deepseekApiKey();

    // 检测 Ollama 本地模型（静默）
    QUrl ollamaApiUrl(ollamaUrl.trimmed().replace(QRegularExpression("/+$"), "") + "/api/tags");
    QNetworkRequest ollamaRequest(ollamaApiUrl);
    ollamaRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *ollamaReply = m_startupNetManager->get(ollamaRequest);
    connect(ollamaReply, &QNetworkReply::finished, this, [this, ollamaReply]() {
        if (ollamaReply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(ollamaReply->readAll());
            QStringList modelList;
            for (const QJsonValue &v : doc.object().value("models").toArray()) {
                QString name = v.toObject().value("name").toString();
                if (!name.isEmpty()) modelList.append(name);
            }
            if (!modelList.isEmpty()) {
                m_settingsManager->setDiscoveredOllamaModels(modelList);
                // 如果当前使用ollama且没有设置模型，自动选择第一个
                if (m_settingsManager->aiProvider() == "ollama" && m_settingsManager->ollamaModel().isEmpty()) {
                    m_settingsManager->setOllamaModel(modelList.first());
                }
            }
        }
        ollamaReply->deleteLater();

        // 继续检测 DeepSeek
        QString deepseekUrl = m_settingsManager->deepseekApiUrl();
        QString deepseekApiKey = m_settingsManager->deepseekApiKey();
        if (!deepseekApiKey.isEmpty()) {
            QUrl dsApiUrl(deepseekUrl.trimmed().replace(QRegularExpression("/+$"), "") + "/models");
            QNetworkRequest dsRequest(dsApiUrl);
            dsRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            dsRequest.setRawHeader("Authorization", QString("Bearer %1").arg(deepseekApiKey).toUtf8());

            QNetworkReply *dsReply = m_startupNetManager->get(dsRequest);
            connect(dsReply, &QNetworkReply::finished, this, [this, dsReply]() {
                if (dsReply->error() == QNetworkReply::NoError) {
                    QJsonDocument doc = QJsonDocument::fromJson(dsReply->readAll());
                    if (doc.object().contains("data") || doc.object().contains("models")) {
                        // DeepSeek API响应成功，保存可用状态
                        m_settingsManager->setDiscoveredDeepseekModels({"deepseek-chat", "deepseek-coder"});
                        if (m_settingsManager->aiProvider() == "deepseek" && m_settingsManager->deepseekModel().isEmpty()) {
                            // 如果当前使用DeepSeek且没有设置模型，使用第一个可用模型
                            QStringList models = doc.object().contains("data")
                                ? parseDeepseekModelsFromData(doc.object().value("data").toArray())
                                : parseDeepseekModelsFromModels(doc.object().value("models").toArray());
                            if (!models.isEmpty()) {
                                m_settingsManager->setDeepseekModel(models.first());
                            }
                        }
                    }
                }
                dsReply->deleteLater();
            });
        }
        m_settingsManager->sync();
    });
}

QStringList MainWindow::parseDeepseekModelsFromData(const QJsonArray &data)
{
    QStringList models;
    for (const QJsonValue &v : data) {
        QString id = v.toObject().value("id").toString();
        if (!id.isEmpty() && id.contains("deepseek", Qt::CaseInsensitive)) {
            models.append(id);
        }
    }
    return models;
}

QStringList MainWindow::parseDeepseekModelsFromModels(const QJsonArray &modelsArr)
{
    QStringList models;
    for (const QJsonValue &v : modelsArr) {
        QString id = v.toObject().value("id").toString();
        if (!id.isEmpty()) models.append(id);
    }
    return models;
}

void MainWindow::sendChatMessage()
{
    const Theme &t = m_darkMode ? DARK : LIGHT;
    QString msg = m_chatInput->toPlainText().trimmed();
    if (msg.isEmpty() || m_isReceiving) return;

    // 如果是初始欢迎页面则清空历史
    static const QString welcomeMarker = "欢迎来到国防知识 AI 助手";
    if (m_chatDisplay->toHtml().contains(welcomeMarker) && m_currentResponse.isEmpty() && !m_isReceiving) {
        m_chatHistory.clear();
    }

    // 添加用户消息到历史
    m_chatHistory.append("user||" + msg);

    // 限制历史条数，防止内存无限增长
    while (m_chatHistory.size() > 100) {
        m_chatHistory.removeFirst();
    }

    // 重建完整聊天显示
    QString fullHtml;
    for (const QString &entry : m_chatHistory) {
        int sep = entry.indexOf("||");
        if (sep < 0) continue;
        QString role = entry.left(sep);
        QString content = entry.mid(sep + 2);
        if (role == "user") {
            fullHtml += QString("<p style='margin:6px 0;'><b style='color:%1;'>您:</b> %2</p>")
                            .arg(t.accent, escapeHtml(content));
        } else {
            fullHtml += QString("<div style='margin:6px 0;color:%1;'><b style='color:%2;'>AI:</b> %3</div>")
                            .arg(t.text, t.accent, content);
        }
    }
    m_chatPrefix = fullHtml;

    // 显示AI正在思考
    m_chatDisplay->setHtml(m_chatPrefix +
        QString("<p style='margin:6px 0;color:%1;'><b>AI:</b> 正在思考...</p>").arg(t.text2));

    m_chatInput->clear();
    m_currentResponse.clear();
    m_isReceiving = true;

    // 根据AI服务商获取模型名称
    QString provider = m_settingsManager->aiProvider();
    QString model = provider == "ollama" ? m_settingsManager->ollamaModel() : m_settingsManager->deepseekModel();
    if (provider == "ollama") {
        m_aiServiceManager->sendMessage(msg, model, m_settingsManager->systemPrompt(), "ollama", m_settingsManager->ollamaUrl());
    } else {
        m_aiServiceManager->sendMessage(msg, model, m_settingsManager->systemPrompt(), "deepseek", m_settingsManager->deepseekApiUrl(), m_settingsManager->deepseekApiKey());
    }

    // 确保所有旧信号已断开，然后连接聊天模式信号
    disconnect(m_aiServiceManager, &AiServiceManager::responseChunk, this, &MainWindow::onQuizResponseChunk);
    disconnect(m_aiServiceManager, &AiServiceManager::responseReceived, this, &MainWindow::onQuizResponseReceived);
    disconnect(m_aiServiceManager, &AiServiceManager::errorOccurred, this, &MainWindow::onQuizError);
    disconnect(m_aiServiceManager, &AiServiceManager::errorOccurred, this, &MainWindow::onErrorOccurred);
    disconnect(m_aiServiceManager, &AiServiceManager::responseChunk, this, &MainWindow::onResponseChunk);
    disconnect(m_aiServiceManager, &AiServiceManager::responseReceived, this, &MainWindow::onResponseReceived);
    connect(m_aiServiceManager, &AiServiceManager::responseChunk, this, &MainWindow::onResponseChunk, Qt::UniqueConnection);
    connect(m_aiServiceManager, &AiServiceManager::responseReceived, this, &MainWindow::onResponseReceived, Qt::UniqueConnection);
    connect(m_aiServiceManager, &AiServiceManager::errorOccurred, this, &MainWindow::onErrorOccurred, Qt::UniqueConnection);
}

QString MainWindow::escapeHtml(const QString &t)
{ QString r=t; r.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;").replace("\"","&quot;"); return r; }

void MainWindow::onAiServiceStatusChanged(bool available, const QString &message)
{
    m_statusDot->setProperty("status", available ? "connected" : "disconnected");
    m_statusDot->style()->unpolish(m_statusDot);
    m_statusDot->style()->polish(m_statusDot);
    m_statusLabel->setProperty("status", available ? "connected" : "disconnected");
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
    m_statusLabel->setText(message);
}

void MainWindow::onResponseChunk(const QString &chunk)
{
    if (!m_isReceiving) return;
    const Theme &t = m_darkMode ? DARK : LIGHT;
    m_currentResponse += chunk;
    QString rendered = renderMarkdown(m_currentResponse);
    m_chatDisplay->setHtml(m_chatPrefix +
        QString("<div style='margin:6px 0;color:%1;'><b style='color:%2;'>AI:</b> %3</div>")
            .arg(t.text, t.accent, rendered));
    QScrollBar *sb = m_chatDisplay->verticalScrollBar(); sb->setValue(sb->maximum());
}

void MainWindow::onResponseReceived(const QString &)
{
    m_isReceiving = false;
    // 将AI回复写入历史
    const Theme &t = m_darkMode ? DARK : LIGHT;
    QString rendered = renderMarkdown(m_currentResponse);
    m_chatHistory.append("ai||" + rendered);
    // 限制历史条数
    while (m_chatHistory.size() > 100) {
        m_chatHistory.removeFirst();
    }
    // 重建完整显示
    QString fullHtml;
    for (const QString &entry : m_chatHistory) {
        int sep = entry.indexOf("||");
        if (sep < 0) continue;
        QString role = entry.left(sep);
        QString content = entry.mid(sep + 2);
        if (role == "user") {
            fullHtml += QString("<p style='margin:6px 0;'><b style='color:%1;'>您:</b> %2</p>")
                            .arg(t.accent, content);
        } else {
            fullHtml += QString("<div style='margin:6px 0;color:%1;'><b style='color:%2;'>AI:</b> %3</div>")
                            .arg(t.text, t.accent, content);
        }
    }
    m_chatPrefix = fullHtml;
    m_chatDisplay->setHtml(fullHtml);
    QScrollBar *sb = m_chatDisplay->verticalScrollBar(); sb->setValue(sb->maximum());
}

void MainWindow::onModelsFetched(const QStringList &models)
{
    // 保存检测到的模型（各服务商各自保存，不依赖当前 provider）
    QString currentText = m_modelSelectCombo->currentText();
    bool isCustom = (m_modelSelectCombo->currentIndex() == m_modelSelectCombo->count() - 1);

    // 从当前请求保存模型到对应服务商
    if (m_settingsManager->aiProvider() == "ollama") {
        m_settingsManager->setDiscoveredOllamaModels(models);
    } else {
        m_settingsManager->setDiscoveredDeepseekModels(models);
    }

    m_modelSelectCombo->clear();
    m_modelSelectCombo->setEditable(true);
    m_modelSelectCombo->lineEdit()->setPlaceholderText("输入或选择模型名称");
    for (const QString &m : models) { m_modelSelectCombo->addItem(m); }
    m_modelSelectCombo->addItem("自定义");

    // 根据当前服务商获取保存的模型
    QString provider = m_settingsManager->aiProvider();
    QString saved = provider == "ollama" ? m_settingsManager->ollamaModel() : m_settingsManager->deepseekModel();

    int idx = m_modelSelectCombo->findText(saved);
    if (idx >= 0) { m_modelSelectCombo->setCurrentIndex(idx); }
    else if (!saved.isEmpty() && models.contains(saved)) {
        m_modelSelectCombo->setCurrentIndex(m_modelSelectCombo->findText(saved));
    } else if (!models.isEmpty()) {
        m_modelSelectCombo->setCurrentIndex(0);
    } else if (isCustom && !currentText.isEmpty() && currentText != "自定义") {
        m_modelSelectCombo->setCurrentText(currentText);
    }

    m_settingsManager->sync();
}

void MainWindow::onErrorOccurred(const QString &error)
{
    m_isReceiving = false;
    // 仅在聊天模式下更新聊天显示，答题模式下由 onQuizError 处理
    if (!m_quizReceiving) {
        m_chatDisplay->setHtml(m_chatPrefix +
            QString("<p style='margin:6px 0;color:#ff4444;'><b>错误:</b> %1</p>").arg(escapeHtml(error)));
    }
}

/* ================================================================== */
/*  知识问答 - 预制的国防知识问答                                      */
/* ================================================================== */
struct QAItem { QString question; QString answer; };

static const QList<QAItem> g_knowledgeQA = {
    {"什么是国防？", "国防是国家为防备和抵抗侵略，制止武装颠覆，保卫国家的主权、统一、领土完整和安全所进行的军事活动，以及与军事有关的政治、经济、外交、科技、教育等方面的活动。国防是国家生存与发展的重要保证。"},
    {"中国国防政策的主要内容是什么？", "中国实行积极防御的军事战略方针，坚持自卫防御的核战略。国防政策的主要内容包括：维护国家主权、安全和发展利益；坚持独立自主的和平外交政策；坚持全民防卫观念；推进国防和军队现代化。"},
    {"什么是人民防空？", "人民防空（简称人防）是国家根据国防需要，动员和组织人民群众采取防护措施，防范和减轻空袭危害的活动。人民防空实行长期准备、重点建设、平战结合的方针，贯彻与经济建设协调发展、与城市建设相结合的原则。"},
    {"中国的武装力量由哪些部分组成？", "中华人民共和国的武装力量由中国人民解放军、中国人民武装警察部队和民兵组成。中国人民解放军现役部队是国家的常备军，由陆军、海军、空军、火箭军等军种组成。"},
    {"什么是全民国防教育日？", "全民国防教育日是每年9月的第三个星期六，是国家为弘扬爱国主义精神、普及国防教育而设立的节日。通过全民国防教育日活动，增强全民国防观念，掌握必要的国防知识和军事技能。"},
    {"中国第一颗原子弹是什么时候爆炸成功的？", "1964年10月16日，中国第一颗原子弹在新疆罗布泊爆炸成功。这标志着中国成为继美国、苏联、英国、法国之后，世界上第五个拥有核武器的国家。"},
    {"什么是预备役？", "预备役是指国家以预备役人员为基础、现役军人为骨干，按战时编制组建的武装力量。预备役人员平时参加生产劳动，定期参加军事训练，战时根据国家动员令服现役。"},
    {"中国人民解放军建军节是哪一天？", "中国人民解放军建军节是每年的8月1日，纪念1927年8月1日南昌起义。南昌起义标志着中国共产党独立领导武装斗争的开始，是人民军队的诞生之日。"},
    {"什么是国防动员？", "国防动员是国家为应对战争或其他安全威胁，使社会诸领域全部或部分由平时状态转入战时状态或紧急状态所进行的活动。包括武装力量动员、国民经济动员、科学技术动员、人民防空动员等。"},
    {"中国航母的发展历程是怎样的？", "2012年9月，辽宁舰（舷号16）交付海军，中国首艘航母入列。2019年12月，山东舰（舷号17）交付海军，中国首艘国产航母服役。2022年6月，福建舰（舷号18）下水，中国首艘电磁弹射型航母诞生。"},
    {"什么是兵役制度？", "兵役制度是国家关于公民参加军队和其他武装组织或在军队外接受军事训练的制度。中国现行兵役制度实行义务兵与志愿兵相结合、民兵与预备役相结合的兵役制度。"},
    {"我国的领海宽度是多少？", "根据《中华人民共和国领海及毗连区法》，中华人民共和国领海宽度为12海里。领海是国家主权范围内的海域，外国非军用船舶享有无害通过权。"}
};

QWidget* MainWindow::createKnowledgeQAPage()
{
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);

    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(24, 20, 24, 20);
    lay->setSpacing(16);

    QLabel *title = new QLabel("国防知识问答");
    title->setObjectName("sectionTitle");
    title->setAlignment(Qt::AlignCenter);
    lay->addWidget(title);

    QLabel *desc = new QLabel("以下为预制的国防知识问答，点击问题即可查看解答");
    desc->setObjectName("dimLabel");
    desc->setAlignment(Qt::AlignCenter);
    lay->addWidget(desc);

    QWidget *qaContainer = new QWidget();
    QVBoxLayout *qaLay = new QVBoxLayout(qaContainer);
    qaLay->setSpacing(12);

    for (int i = 0; i < g_knowledgeQA.size(); ++i) {
        const QAItem &item = g_knowledgeQA[i];

        QFrame *qCard = new QFrame();
        qCard->setObjectName("equipCard");
        qCard->setCursor(Qt::PointingHandCursor);
        qCard->setProperty("qaCard", true);
        qCard->setProperty("qaIdx", i);
        qCard->installEventFilter(this);

        QVBoxLayout *cardLay = new QVBoxLayout(qCard);
        cardLay->setSpacing(12);
        cardLay->setContentsMargins(20, 16, 20, 16);

        QLabel *qLabel = new QLabel(item.question);
        qLabel->setObjectName("equipName");
        qLabel->setWordWrap(true);
        qLabel->setCursor(Qt::PointingHandCursor);
        cardLay->addWidget(qLabel);

        QLabel *aLabel = new QLabel(item.answer);
        aLabel->setObjectName(QString("qaAnswer%1").arg(i));
        aLabel->setWordWrap(true);
        aLabel->setStyleSheet("font-size: 15px; line-height: 1.6;");
        aLabel->setVisible(false);
        cardLay->addWidget(aLabel);

        qaLay->addWidget(qCard);
    }

    lay->addWidget(qaContainer);
    lay->addStretch();

    scroll->setWidget(page);
    return scroll;
}

/* ================================================================== */
/*  知识答题 - 20道预制答题                                          */
/* ================================================================== */

static const QList<QVariantMap> g_knowledgeQuizQuestions = [](){
    QList<QVariantMap> list;
    auto addQ = [&](const QString &q, const QStringList &opts, int ans, const QString &exp) {
        QVariantMap m;
        m["question"] = q;
        m["options"] = opts;
        m["answer"] = ans;
        m["explanation"] = exp;
        list.append(m);
    };

    addQ("中国人民解放军建军节是哪一天？",
         {"7月1日", "8月1日", "10月1日", "12月25日"}, 1,
         "1927年8月1日南昌起义标志着中国共产党独立领导武装斗争的开始。");

    addQ("中国第一颗原子弹爆炸成功是在哪一年？",
         {"1960年", "1962年", "1964年", "1966年"}, 2,
         "1964年10月16日，中国第一颗原子弹爆炸成功。");

    addQ("中国首艘航空母舰辽宁舰是哪一年交付海军的？",
         {"2010年", "2011年", "2012年", "2013年"}, 2,
         "2012年9月，辽宁舰（舷号16）交付海军。");

    addQ("我国领海宽度为多少海里？",
         {"3海里", "12海里", "24海里", "200海里"}, 1,
         "根据相关法律规定，我国领海宽度为12海里。");

    addQ("全民国防教育日是每年几月的第几个星期六？",
         {"8月第一个星期六", "9月第三个星期六", "10月第二个星期六", "11月第四个星期六"}, 1,
         "全民国防教育日是每年9月的第三个星期六。");

    addQ("中国的武装力量由哪三部分组成？",
         {"解放军、武警、民兵", "解放军、警察、民兵", "解放军、武警、预备役", "陆军、海军、空军"}, 0,
         "中国武装力量由中国人民解放军、中国人民武装警察部队和民兵组成。");

    addQ("东风-41洲际弹道导弹的射程大约是多少公里？",
         {"8000公里", "10000公里", "14000公里以上", "20000公里"}, 2,
         "东风-41射程超过14000公里，可覆盖全球大部分地区。");

    addQ("歼-20是中国自主研制的第几代战斗机？",
         {"第三代", "第四代", "第五代", "第六代"}, 2,
         "歼-20是第五代战斗机，采用鸭式气动布局，具备超音速巡航能力。");

    addQ("福建舰是中国第几艘电磁弹射型航空母舰？",
         {"第一艘", "第二艘", "第三艘", "第四艘"}, 0,
         "福建舰（舷号18）是中国完全自主设计的首艘弹射型航母。");

    addQ("99A主战坦克的主炮口径是多少毫米？",
         {"100毫米", "105毫米", "120毫米", "125毫米"}, 3,
         "99A搭载125mm滑膛炮，可发射炮射导弹。");

    addQ("055型驱逐舰的满载排水量约为多少吨？",
         {"8000吨", "10000吨", "12500吨", "15000吨"}, 2,
         "055型满载排水量超12000吨，是世界最先进的驱逐舰之一。");

    addQ("中国现行兵役制度实行的是什么制度？",
         {"纯义务兵役制", "纯志愿兵役制", "义务兵与志愿兵相结合", "雇佣兵役制"}, 2,
         "中国实行义务兵与志愿兵相结合、民兵与预备役相结合的兵役制度。");

    addQ("中国成为世界上第几个拥有核武器的国家？",
         {"第三个", "第四个", "第五个", "第六个"}, 2,
         "中国是继美、苏、英、法之后第五个拥有核武器的国家。");

    addQ("空警-500预警机是以哪种飞机为平台研制的？",
         {"运-8", "运-9", "运-20", "轰-6"}, 1,
         "空警-500以运-9为平台，搭载数字阵列有源相控阵雷达。");

    addQ("运-20大型运输机的最大载重约为多少吨？",
         {"44吨", "55吨", "66吨", "77吨"}, 2,
         "运-20（鲲鹏）最大载重约66吨。");

    addQ("东风-17高超音速导弹的最大速度超过多少马赫？",
         {"5马赫", "8马赫", "10马赫", "15马赫"}, 2,
         "东风-17速度超过10马赫，弹道不可预测，现有反导系统难以拦截。");

    addQ("红旗-9B防空导弹的最大射程约为多少公里？",
         {"100公里", "200公里", "300公里", "500公里"}, 2,
         "红旗-9B最大射程约300公里，是国土防空体系核心装备。");

    addQ("国防动员的核心目的是什么？",
         {"发展经济", "应对战争或其他安全威胁", "促进国际合作", "维护社会稳定"}, 1,
         "国防动员是国家为应对战争或其他安全威胁所进行的活动。");

    addQ("中国实行积极防御的军事战略方针，其核心是什么？",
         {"先发制人", "自卫防御", "进攻为主", "威慑为主"}, 1,
         "中国坚持积极防御的军事战略方针，核心是自卫防御。");

    addQ("人民防空实行的方针是什么？",
         {"战时准备、全面建设、军民结合", "长期准备、重点建设、平战结合", "应急准备、全面覆盖、军民融合", "预防为主、防治结合、全民参与"}, 1,
         "人民防空实行长期准备、重点建设、平战结合的方针。");

    return list;
}();

QWidget* MainWindow::createKnowledgeQuizPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(20, 16, 20, 16);
    lay->setSpacing(10);

    QLabel *title = new QLabel("🎯 国防知识答题");
    title->setObjectName("sectionTitle");
    title->setAlignment(Qt::AlignCenter);
    lay->addWidget(title);

    QLabel *desc = new QLabel("预制20道国防知识题目，测试你的国防知识水平！");
    desc->setObjectName("dimLabel");
    desc->setAlignment(Qt::AlignCenter);
    lay->addWidget(desc);

    // 开始答题按钮
    QFrame *cfg = new QFrame();
    cfg->setObjectName("quizCfg");
    QHBoxLayout *cl = new QHBoxLayout(cfg);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->addStretch();
    m_kQuizScoreLabel->setObjectName("quizScoreLabel");
    m_kQuizScoreLabel->setText("得分: -");
    cl->addWidget(m_kQuizScoreLabel);
    QPushButton *startBtn = new QPushButton("开始答题");
    startBtn->setObjectName("primaryBtn");
    startBtn->setFixedHeight(28);
    connect(startBtn, &QPushButton::clicked, this, &MainWindow::startKnowledgeQuiz);
    cl->addWidget(startBtn);
    lay->addWidget(cfg);

    m_kQuizProgress->setObjectName("quizProgress");
    m_kQuizProgress->setText("点击「开始答题」开始");
    m_kQuizProgress->setAlignment(Qt::AlignCenter);
    lay->addWidget(m_kQuizProgress);

    m_kQuizQuestion->setObjectName("quizQuestion");
    m_kQuizQuestion->setReadOnly(true);
    m_kQuizQuestion->setHtml("<p style='color:#888;text-align:center;'>等待开始...</p>");
    m_kQuizQuestion->setMinimumHeight(100);
    lay->addWidget(m_kQuizQuestion);

    QFrame *ansF = new QFrame();
    QVBoxLayout *ansL = new QVBoxLayout(ansF);
    ansL->setSpacing(6);
    ansL->setContentsMargins(0, 0, 0, 0);
    for (int i = 0; i < 4; ++i) {
        QPushButton *btn = new QPushButton();
        btn->setMinimumHeight(40);
        btn->setObjectName("ansBtn");
        btn->setVisible(false);
        int idx = i;
        connect(btn, &QPushButton::clicked, this, [this, idx]() { selectKAnswer(idx); });
        ansL->addWidget(btn);
        m_kQuizButtons.append(btn);
    }
    lay->addWidget(ansF);

    m_kQuizFeedback->setObjectName("feedbackLabel");
    m_kQuizFeedback->setAlignment(Qt::AlignCenter);
    m_kQuizFeedback->hide();
    lay->addWidget(m_kQuizFeedback);

    m_kQuizNextBtn = new QPushButton("下一题 →");
    m_kQuizNextBtn->setObjectName("kQuizNextBtn");
    m_kQuizNextBtn->hide();
    connect(m_kQuizNextBtn, &QPushButton::clicked, this, &MainWindow::nextKQuestion);
    lay->addWidget(m_kQuizNextBtn, 0, Qt::AlignCenter);

    lay->addStretch();
    return page;
}

/* ---- 知识答题逻辑 ---- */
void MainWindow::startKnowledgeQuiz()
{
    m_kQuizQuestions = g_knowledgeQuizQuestions;
    m_kQuizTotal = m_kQuizQuestions.size();
    m_kQuizCurrent = 0;
    m_kQuizCorrect = 0;
    m_kQuizSelectedAnswer = -1;

    m_kQuizProgress->setText("正在答题...");
    m_kQuizScoreLabel->setText("得分: 0");
    m_kQuizFeedback->hide();
    for (auto b : m_kQuizButtons) b->setVisible(false);
    m_kQuizNextBtn->hide();

    showCurrentKQuizQuestion();
}

void MainWindow::showCurrentKQuizQuestion()
{
    if (m_kQuizCurrent >= m_kQuizQuestions.size()) { finishKQuiz(); return; }
    const QVariantMap &q = m_kQuizQuestions[m_kQuizCurrent];
    m_kQuizProgress->setText(QString("第 %1 / %2 题").arg(m_kQuizCurrent + 1).arg(m_kQuizTotal));
    m_kQuizQuestion->setHtml(QString("<p style='font-size:15px;'>%1</p>").arg(escapeHtml(q.value("question").toString())));
    QStringList opts = q.value("options").toStringList();
    QStringList labels = {"A", "B", "C", "D"};
    for (int i = 0; i < 4; ++i) {
        if (i < opts.size()) {
            m_kQuizButtons[i]->setText(labels[i] + ". " + opts[i]);
            m_kQuizButtons[i]->setVisible(true);
            m_kQuizButtons[i]->setEnabled(true);
            m_kQuizButtons[i]->setProperty("correct", QVariant());
            m_kQuizButtons[i]->setProperty("wrong", QVariant());
            m_kQuizButtons[i]->setProperty("dim", QVariant());
            m_kQuizButtons[i]->style()->unpolish(m_kQuizButtons[i]);
            m_kQuizButtons[i]->style()->polish(m_kQuizButtons[i]);
        } else {
            m_kQuizButtons[i]->setVisible(false);
        }
    }
    m_kQuizFeedback->hide();
    m_kQuizNextBtn->hide();
    m_kQuizSelectedAnswer = -1;
}

void MainWindow::selectKAnswer(int idx)
{
    if (m_kQuizSelectedAnswer >= 0) return;
    m_kQuizSelectedAnswer = idx;
    const QVariantMap &q = m_kQuizQuestions[m_kQuizCurrent];
    int correct = q.value("answer").toInt();
    QString explanation = q.value("explanation").toString();

    for (int i = 0; i < 4; ++i) {
        m_kQuizButtons[i]->setEnabled(false);
        m_kQuizButtons[i]->setProperty("correct", QVariant());
        m_kQuizButtons[i]->setProperty("wrong", QVariant());
        m_kQuizButtons[i]->setProperty("dim", QVariant());

        if (i == correct)
            m_kQuizButtons[i]->setProperty("correct", true);
        else if (i == idx && idx != correct)
            m_kQuizButtons[i]->setProperty("wrong", true);
        else
            m_kQuizButtons[i]->setProperty("dim", true);

        m_kQuizButtons[i]->style()->unpolish(m_kQuizButtons[i]);
        m_kQuizButtons[i]->style()->polish(m_kQuizButtons[i]);
    }

    bool ok = (idx == correct);
    if (ok) {
        m_kQuizCorrect++;
        m_kQuizFeedback->setStyleSheet("color:#00cc66;");
        m_kQuizFeedback->setText("✓ 正确！");
    } else {
        m_kQuizFeedback->setStyleSheet("color:#ff4444;");
        m_kQuizFeedback->setText("✗ 错误");
    }
    m_kQuizFeedback->show();
    if (!explanation.isEmpty())
        m_kQuizFeedback->setText(m_kQuizFeedback->text() + "  " + explanation);
    m_kQuizScoreLabel->setText(QString("得分: %1/%2").arg(m_kQuizCorrect).arg(m_kQuizCurrent + 1));

    m_kQuizNextBtn->setText(m_kQuizCurrent + 1 < m_kQuizQuestions.size() ? "下一题 →" : "查看结果");
    m_kQuizNextBtn->show();
}

void MainWindow::nextKQuestion()
{
    m_kQuizCurrent++;
    showCurrentKQuizQuestion();
}

void MainWindow::finishKQuiz()
{
    const Theme &t = m_darkMode ? DARK : LIGHT;
    int pct = m_kQuizTotal > 0 ? m_kQuizCorrect * 100 / m_kQuizTotal : 0;
    QString grade = pct >= 90 ? "🏆 优秀！" : pct >= 70 ? "👍 良好" : pct >= 50 ? "📖 还需努力" : "💪 再接再厉";
    m_kQuizQuestion->setHtml(QString(
        "<div style='text-align:center;margin:20px;'>"
        "<p style='font-size:36px;font-weight:bold;color:%1;'>%2 / %3</p>"
        "<p style='font-size:16px;color:%4;'>%5</p></div>")
        .arg(t.accent).arg(m_kQuizCorrect).arg(m_kQuizTotal).arg(t.text).arg(grade));
    m_kQuizProgress->setText("答题完成！");
    m_kQuizScoreLabel->setText(QString("最终: %1/%2").arg(m_kQuizCorrect).arg(m_kQuizTotal));
    for (auto b : m_kQuizButtons) b->setVisible(false);
    m_kQuizFeedback->hide();
    m_kQuizNextBtn->setText("再来一轮");
    m_kQuizNextBtn->show();
    disconnect(m_kQuizNextBtn, &QPushButton::clicked, this, &MainWindow::nextKQuestion);
    connect(m_kQuizNextBtn, &QPushButton::clicked, this, &MainWindow::startKnowledgeQuiz);
}

/* ================================================================== */
/*  设置逻辑                                                          */
/* ================================================================== */
void MainWindow::saveSettings()
{
    m_settingsManager->setTheme(m_themeMode);
    m_settingsManager->setAcrylicMode(m_acrylicMode);
    m_settingsManager->setSidebarAutoCollapse(m_sidebarAutoCollapse);

    int providerIdx = m_aiProviderCombo->currentIndex();
    m_settingsManager->setAiProvider(providerIdx == 0 ? "ollama" : "deepseek");
    m_settingsManager->setOllamaUrl(m_urlInput->text());

    QString ollamaModelText = m_modelSelectCombo->currentText();
    if (ollamaModelText == "自定义") {
        ollamaModelText = "";  // 不保存字面量"自定义"
    }
    m_settingsManager->setOllamaModel(ollamaModelText);

    QComboBox *deepseekModelCombo = m_aiSettingsStack->widget(1)->findChild<QComboBox*>("deepseekModelCombo");
    QString deepseekModelText = deepseekModelCombo ? deepseekModelCombo->currentText() : "deepseek-chat";
    m_settingsManager->setDeepseekModel(deepseekModelText);

    m_settingsManager->setDeepseekApiUrl(m_deepseekApiUrlInput->text());
    m_settingsManager->setDeepseekApiKey(m_deepseekApiKeyInput->text());

    m_settingsManager->setSystemPrompt(m_promptInput->toPlainText());
    m_settingsManager->sync();
}

void MainWindow::debouncedSaveSettings()
{
    m_saveDebounceTimer->start();
}

void MainWindow::loadSettingsToUi()
{
    m_themeMode = m_settingsManager->theme();
    m_acrylicMode = m_settingsManager->acrylicMode();
    m_sidebarAutoCollapse = m_settingsManager->sidebarAutoCollapse();
    if (m_themeMode == "system") {
        m_darkMode = systemDarkMode();
        m_autoBtn->setChecked(true); m_darkBtn->setChecked(false); m_lightBtn->setChecked(false);
    } else if (m_themeMode == "dark") {
        m_darkMode = true;
        m_autoBtn->setChecked(false); m_darkBtn->setChecked(true); m_lightBtn->setChecked(false);
    } else {
        m_darkMode = false;
        m_autoBtn->setChecked(false); m_darkBtn->setChecked(false); m_lightBtn->setChecked(true);
    }

    // 加载AI服务商设置
    if (m_aiProviderCombo && m_aiSettingsStack) {
        QString provider = m_settingsManager->aiProvider();
        m_aiProviderCombo->setCurrentIndex(provider == "ollama" ? 0 : 1);
        m_aiSettingsStack->setCurrentIndex(provider == "ollama" ? 0 : 1);
    }

    m_urlInput->setText(m_settingsManager->ollamaUrl());

    // 加载 Ollama 已检测的模型到下拉框
    QStringList ollamaModels = m_settingsManager->discoveredOllamaModels();
    m_modelSelectCombo->clear();
    for (const QString &m : ollamaModels) { m_modelSelectCombo->addItem(m); }
    m_modelSelectCombo->addItem("自定义");

    // 设置当前选中的 Ollama 模型
    QString savedOllamaModel = m_settingsManager->ollamaModel();
    int ollamaIdx = m_modelSelectCombo->findText(savedOllamaModel);
    if (ollamaIdx >= 0) {
        m_modelSelectCombo->setCurrentIndex(ollamaIdx);
    } else if (!savedOllamaModel.isEmpty() && !ollamaModels.isEmpty()) {
        // Saved model not in detected list, use custom
        m_modelSelectCombo->setCurrentIndex(m_modelSelectCombo->count() - 1);
    }

    // 加载 DeepSeek 模型
    QComboBox *deepseekModelCombo = m_aiSettingsStack->widget(1)->findChild<QComboBox*>("deepseekModelCombo");
    if (deepseekModelCombo) {
        QString savedDeepseekModel = m_settingsManager->deepseekModel();
        int dsIdx = deepseekModelCombo->findText(savedDeepseekModel);
        if (dsIdx >= 0) {
            deepseekModelCombo->setCurrentIndex(dsIdx);
        }
    }

    if (m_deepseekApiUrlInput) m_deepseekApiUrlInput->setText(m_settingsManager->deepseekApiUrl());
    if (m_deepseekApiKeyInput) m_deepseekApiKeyInput->setText(m_settingsManager->deepseekApiKey());

    m_promptInput->setPlainText(m_settingsManager->systemPrompt());

    // Update acrylic and sidebar checkboxes via findChild
    QWidget *settingsPage = m_stack->widget(7); // settings page is index 7
    if (settingsPage) {
        QCheckBox *acrylicCheck = settingsPage->findChild<QCheckBox*>("acrylicCheck");
        if (acrylicCheck) acrylicCheck->setChecked(m_acrylicMode);
        QCheckBox *sidebarCheck = settingsPage->findChild<QCheckBox*>("sidebarAutoCheck");
        if (sidebarCheck) sidebarCheck->setChecked(m_sidebarAutoCollapse);
    }
}

void MainWindow::resetSettings()
{
    if (QMessageBox::question(this, QString::fromUtf8("确认"),
                              QString::fromUtf8("确定恢复默认设置？")) == QMessageBox::Yes) {
        m_settingsManager->setTheme("system"); m_settingsManager->setFontSize(14);
        m_settingsManager->setAiProvider("ollama");
        m_settingsManager->setOllamaUrl("http://localhost:11434");
        m_settingsManager->setOllamaModel("");  // 空，等待启动时自动检测
        m_settingsManager->setDeepseekApiUrl("https://api.deepseek.com/v1");
        m_settingsManager->setDeepseekApiKey("");
        m_settingsManager->setDeepseekModel("");  // 空，等待启动时自动检测
        m_settingsManager->setSystemPrompt(SettingsManager::defaultSystemPrompt());
        loadSettingsToUi(); applyTheme();
        QMessageBox::information(this, QString::fromUtf8("已恢复"), QString::fromUtf8("设置已恢复！"));
    }
}

void MainWindow::showChangelog()
{
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("更新日志");
    dlg->setMinimumSize(520, 500);

    QVBoxLayout *lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(24, 20, 24, 20);
    lay->setSpacing(16);

    QLabel *title = new QLabel("更新日志");
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter);
    lay->addWidget(title);

    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);

    QWidget *content = new QWidget();
    QVBoxLayout *cl = new QVBoxLayout(content);
    cl->setSpacing(16);

    // 尝试读取本地changelog.md文件
    QString changelogPath = QApplication::applicationDirPath() + "/changelog.md";
    QFile changelogFile(changelogPath);
    QString changelogText;

    if (changelogFile.exists() && changelogFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        changelogText = QString::fromUtf8(changelogFile.readAll());
        changelogFile.close();
    } else {
        // 如果本地文件不存在，尝试从资源文件读取
        QFile resFile(":/resources/changelog.md");
        if (resFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            changelogText = QString::fromUtf8(resFile.readAll());
            resFile.close();
        }
    }

    // 解析Markdown并显示
    if (!changelogText.isEmpty()) {
        // 逐行解析Markdown，转换为HTML
        QStringList lines = changelogText.split('\n');
        QString html;

        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];

            // 一级标题
            if (line.startsWith("# ")) {
                html += QString("<h1 style='color: #ffd700; font-size: 20px; text-align: center;'>%1</h1>")
                    .arg(line.mid(2).toHtmlEscaped());
            }
            // 二级标题
            else if (line.startsWith("## ")) {
                html += QString("<h2 style='color: #ffd700; font-size: 16px; margin-top: 16px;'>%1</h2>")
                    .arg(line.mid(3).toHtmlEscaped());
            }
            // 三级标题
            else if (line.startsWith("### ")) {
                html += QString("<h3 style='color: #ffd700; font-size: 14px;'>%1</h3>")
                    .arg(line.mid(4).toHtmlEscaped());
            }
            // 分隔线
            else if (line == "---" || line == "***") {
                html += "<hr style='border: none; border-top: 1px solid #333; margin: 12px 0;'>";
            }
            // 空行
            else if (line.trimmed().isEmpty()) {
                html += "<br>";
            }
            // 列表项 (- 或 *)
            else if (line.startsWith("- ") || line.startsWith("* ")) {
                QString content = line.mid(2);
                // 先转义HTML，再处理粗体（避免 <b> 被转义）
                content = content.toHtmlEscaped();
                content.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>");
                html += QString("&bull; %1<br>").arg(content);
            }
            // 普通文本
            else {
                // 先转义HTML，再处理粗体
                line = line.toHtmlEscaped();
                line.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>");
                html += line + "<br>";
            }
        }

        QLabel *changelogLabel = new QLabel(html);
        changelogLabel->setObjectName("dimLabel");
        changelogLabel->setWordWrap(true);
        changelogLabel->setTextFormat(Qt::RichText);
        cl->addWidget(changelogLabel);
    } else {
        QLabel *noDataLabel = new QLabel("未找到更新日志文件");
        noDataLabel->setObjectName("dimLabel");
        noDataLabel->setAlignment(Qt::AlignCenter);
        cl->addWidget(noDataLabel);
    }

    cl->addStretch();
    scroll->setWidget(content);
    lay->addWidget(scroll, 1);

    QPushButton *closeBtn = new QPushButton("关  闭");
    closeBtn->setObjectName("primaryBtn");
    closeBtn->setFixedWidth(100);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    lay->addWidget(closeBtn, 0, Qt::AlignCenter);

    dlg->exec();
    dlg->deleteLater();
}

/* ================================================================== */
/*  教程辅助方法                                                        */
/* ================================================================== */
void MainWindow::startTutorial()
{
    TutorialDialog *tutorial = new TutorialDialog(this);
    connect(tutorial, &TutorialDialog::tutorialFinished, tutorial, &TutorialDialog::deleteLater);
    tutorial->start();
}

void MainWindow::openSidebarForTutorial()
{
    if (!m_drawerOpen) {
        openSidebar();
    }
}

void MainWindow::showPageForTutorial(int index)
{
    if (index >= 0 && index < m_stack->count()) {
        showPage(index);
    }
}
