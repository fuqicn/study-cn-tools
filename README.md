# 国防安全科普教育软件 v3.2.3

一款国防安全科普软件，使用 C++ 和 Qt 6 开发，拥有精美界面与交互动画。

**制作者：傅琪**

**声明：本软件基于 Qt 开发。Qt 不用于任何公司和商业用途。**

---

## 功能特点

### 1. 国防装备
- 展示中国自主研发的九大国防装备
- 包含歼-20、福建舰、东风-41等
- 点击卡片查看详细参数与介绍

### 2. 发展历程
- 数轴式时间轴，支持拖动浏览
- 回顾过去（南昌起义、新中国成立、抗美援朝）
- 立足现在（辽宁舰、歼-20、东风-41、福建舰）
- 展望未来（现代化军队、世界一流军队）
- 中心放大、边缘缩小的视觉层次效果
- 鼠标滚轮缩放，拖拽平移

### 3. AI 智能问答
- 集成 Ollama / DeepSeek AI 服务
- 预制国防知识专属提示词
- 流式响应实时显示
- 自动检测可用模型
- 快捷提问按钮

### 4. AI 智能答题
- AI 自动生成国防知识选择题
- 可选 3/5/10 题量
- 即时判分与答案解析
- 答题完成评级

### 5. 知识问答 & 知识答题
- 预制 12 道国防知识问答题
- 预制 20 道国防知识选择题
- 查看详细解答

### 6. 激光防御模拟
- 纯平面 2D 炮塔防御小游戏
- 炮管自动朝向鼠标，有旋转速度上限
- 按住鼠标持续发射激光
- 波次递增，难度逐渐上升

### 7. 软件设置
- 主题切换：跟随系统 / 深色 / 浅色
- 亚克力效果开关（Win11 毛玻璃背景）
- 侧边栏自动收回选项
- AI 服务商选择（Ollama / DeepSeek）
- 设置持久化保存到程序目录 `settings.ini`
- 查看更新日志

---

## 编译要求

- Qt 6.x (Widgets, Network, Svg 模块)
- MinGW-w64 编译器
- Windows 10/11

## 编译 & 部署

```batch
cd code
mkdir build && cd build
qmake ..\defense-edu.pro -spec win32-g++ "CONFIG+=release"
mingw32-make -f Makefile.Release
```

或直接双击运行 `build.bat` 一键编译 + 部署到 `../appfolder/`。

部署依赖：
```batch
windeployqt --release --no-translations ..\..\appfolder\defense-edu.exe
```

## 制作安装包

使用 Inno Setup 6 编译 `installer.iss`：

```batch
ISCC.exe installer.iss
```

安装包输出到 `../installer_output/`。安装程序仅当前用户安装（`PrivilegesRequired=lowest`），默认路径 `%LOCALAPPDATA%\Programs\国防安全科普教育软件`。

---

## 运行要求

- **Windows 10 或 Windows 11**（基于 Qt 6，不支持 Windows 7）
- 显卡支持 OpenGL（软件渲染回退可用）

### 可选（用于 AI 功能）
- Ollama 本地服务 (https://ollama.ai)
- 或 DeepSeek API 密钥

## 项目结构

```
defense-edu/
├── code/                       # 源代码与项目配置
│   ├── src/                   # C++ 源代码
│   │   ├── main.cpp          # 程序入口
│   │   ├── mainwindow.cpp/h  # 主窗口
│   │   ├── aiservicemanager.cpp/h   # AI 服务管理
│   │   ├── settingsmanager.cpp/h    # 设置管理
│   │   ├── timelinewidget.cpp/h     # 数轴时间轴控件
│   │   ├── gamewidget.cpp/h  # 激光防御游戏控件
│   │   ├── firstrundialog.cpp/h     # 首次运行向导
│   │   └── tutorialdialog.cpp/h     # 交互式教程
│   ├── resources/            # 资源文件
│   │   ├── resources.qrc    # Qt 资源
│   │   ├── app.rc           # Windows 版本信息
│   │   ├── changelog.md     # 更新日志
│   │   └── icon.ico         # 程序图标
│   ├── build/               # 编译输出目录（生成的）
│   ├── installer.iss        # Inno Setup 安装脚本
│   └── defense-edu.pro      # Qt 项目文件
├── appfolder/                # 部署目录（含 Qt 运行时）
├── installer_output/         # 安装包输出目录
├── build.bat                 # 一键编译部署脚本
└── README.md                 # 本文件
```

## 技术栈

- **语言**: C++17 / Qt 6
- **界面**: Qt Widgets 自绘
- **AI 接口**: Ollama REST API / DeepSeek API
- **编译**: qmake + g++ (MinGW)

## 许可证

本软件用于国防科普教育目的。Qt 采用开源协议使用，不用于任何公司和商业用途。
