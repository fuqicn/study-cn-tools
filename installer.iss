; 国防安全科普教育软件 - Inno Setup 安装脚本
; 制作者：傅琪

#define AppName "国防安全科普教育软件"
#define AppVersion "3.2.1"
#define AppPublisher "傅琪"
#define AppExeName "defense-edu.exe"
#define AppIcon "resources\icon.ico"

[Setup]
AppId={{B8E3D7A1-5C2F-4A91-9D8E-6F0B1A2C3D4E}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
DefaultGroupName={#AppName}
DefaultDirName={autopf}\{#AppName}
PrivilegesRequired=lowest
UninstallDisplayName={#AppName}
UninstallDisplayIcon={app}\{#AppExeName}
OutputDir=..\installer_output
OutputBaseFilename=DefenseEdu_Setup_{#AppVersion}
SetupIconFile={#AppIcon}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
Uninstallable=yes
LicenseFile=LICENSE.txt
VersionInfoVersion={#AppVersion}
VersionInfoCompany={#AppPublisher}
VersionInfoProductName={#AppName}

[Messages]
; --- 覆盖默认英文消息为中文 ---
SetupAppTitle=安装 - {#AppName}
SetupWindowTitle=安装 - {#AppName}
WelcomeLabel1=欢迎使用 {#AppName} 安装向导
WelcomeLabel2=这将安装 {#AppName} 到您的计算机。%n%n本软件集成了国防装备展示、时间轴、AI 智能问答、AI 答题、知识问答、知识答题、激光防御模拟等功能。%n%n制作者：傅琪%n%n声明：Qt 不用于任何公司和商业用途。%n%n建议在继续之前关闭所有其他应用程序。%n%n版本 3.2.1 改进：单实例自动唤醒窗口、激光防御控制按钮、AI 输出 Markdown 渲染。
SelectDirLabel3=安装程序将把 {#AppName} 安装到以下文件夹。
SelectDirBrowseLabel=如需安装到其他文件夹，请点击「浏览」。%n%n选择安装范围：
SelectProgramGroupLabel2=安装程序将在以下开始菜单文件夹中创建快捷方式。
ReadyLabel1=安装程序已准备好将 {#AppName} 安装到您的计算机。
ReadyLabel2a=点击「安装」开始安装，或点击「上一步」修改设置。
InstallingLabel=正在安装 {#AppName}，请稍候...
FinishedHeadingLabel={#AppName} 安装完成
FinishedLabelNoIcons={#AppName} 已成功安装到您的计算机。
FinishedLabel={#AppName} 已成功安装到您的计算机。
ClickFinish=点击「完成」退出安装向导。
LicenseLabel3=请在安装之前阅读以下声明。如果您接受该声明，请点击「我同意」继续。
LicenseAccepted=我同意此声明(&A)
LicenseNotAccepted=我不同意此声明
WizardLicense=声明
WizardSelectDir=选择安装目录
WizardSelectProgramGroup=选择开始菜单文件夹
WizardReady=准备安装
WizardInstalling=正在安装
WizardFinished=安装完成
ButtonNext=下一步(&N) >
ButtonInstall=安装(&I)
ButtonBack=< 上一步(&B)
ButtonCancel=取消
ButtonFinish=完成(&F)
ButtonBrowse=浏览(&R)...
SelectDirDesc=选择 {#AppName} 的安装目录
SelectProgramGroupDesc=选择开始菜单文件夹
ReadyMemoDir=安装目录:
ReadyMemoGroup=开始菜单文件夹:
ReadyMemoTasks=附加任务:

[CustomMessages]
DesktopIcon=创建桌面快捷方式
RunAfterInstall=安装完成后运行 {#AppName}

[Tasks]
Name: "desktopicon"; Description: "{cm:DesktopIcon}"; GroupDescription: "附加快捷方式:"
Name: "runafterinstall"; Description: "{cm:RunAfterInstall}"; GroupDescription: "其他选项:"

[Files]
; 主程序
Source: "..\appfolder\defense-edu.exe"; DestDir: "{app}"; Flags: ignoreversion
; Qt DLL
Source: "..\appfolder\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\Qt6Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\Qt6Svg.dll"; DestDir: "{app}"; Flags: ignoreversion
; MinGW runtime
Source: "..\appfolder\libgcc_s_seh-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion
; OpenGL / software rendering
Source: "..\appfolder\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\D3Dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion
; Qt 插件目录
Source: "..\appfolder\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs
Source: "..\appfolder\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs
Source: "..\appfolder\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs
Source: "..\appfolder\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs
Source: "..\appfolder\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs
Source: "..\appfolder\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs
Source: "..\appfolder\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs
; README 和声明
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{group}\卸载 {#AppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:RunAfterInstall}"; Flags: nowait postinstall skipifsilent; Tasks: runafterinstall

[Code]
var
  FeaturePage: TOutputMsgMemoWizardPage;

procedure InitializeWizard;
begin
  // 在「准备安装」页面之前，插入功能介绍页面
  FeaturePage := CreateOutputMsgMemoPage(
    wpReady,
    '软件功能介绍',
    '了解 {#AppName} 的各项功能',
    '',
    ''
  );

  FeaturePage.RichEditViewer.RTFText :=
    '{\rtf1\ansi\ansicpg936\deff0\nouicompat\deflang1033\deflangfe2052' +
    '{\fonttbl{\f0\fnil\fcharset134 Microsoft YaHei;}}' +
    '{\colortbl;\red196\green30\blue58;\red255\green215\blue0;\red0\green102\blue204;}' +
    '\viewkind4\uc1\pard\sl276\slmult1\f0\fs22' +

    '\pard\qc\b\fs28\cf1 国防安全科普教育软件 v3.2.1\cf0\b0\fs22\par' +
    '\pard\qc\fs18\cf2 制作者：傅琪\par' +
    '\pard\qc\fs16\cf0 声明：Qt 不用于任何公司和商业用途\par' +
    '\pard\sl360\slmult1\fs20\par' +

    '\pard\b\fs24\cf3 1. 国防装备\cf0\b0\fs20\par' +
    '展示中国自主研发的九大国防装备，包括歼-20、福建舰、东风-41等。\par' +
    '点击卡片可查看详细参数与介绍。\par' +
    '\par' +

    '\pard\b\fs24\cf3 2. 发展历程\cf0\b0\fs20\par' +
    '数轴式时间轴，拖动浏览从南昌起义到未来愿景的完整历程。\par' +
    '中心放大、边缘缩小的视觉层次效果，滚轮缩放。\par' +
    '\par' +

    '\pard\b\fs24\cf3 3. AI 智能问答\cf0\b0\fs20\par' +
    '集成 Ollama 本地大模型，预制国防知识专属提示词。\par' +
    '流式响应实时显示，自动检测服务与模型状态。\par' +
    '\par' +

    '\pard\b\fs24\cf3 4. AI 智能答题\cf0\b0\fs20\par' +
    'AI 自动生成国防知识选择题，可选3/5/10题。\par' +
    '即时判分与答案解析，答题完成评级。\par' +
    '\par' +

    '\pard\b\fs24\cf3 5. 知识问答\cf0\b0\fs20\par' +
    '预制12道国防知识问答，涵盖国防政策、兵役制度、航母发展等。\par' +
    '\par' +

    '\pard\b\fs24\cf3 6. 知识答题\cf0\b0\fs20\par' +
    '预制20道国防知识选择题，测试您的国防知识水平。\par' +
    '\par' +

    '\pard\b\fs24\cf3 7. 激光防御模拟\cf0\b0\fs20\par' +
    '2D炮塔防御小游戏：炮管朝向鼠标，按住鼠标持续发射激光。\par' +
    '消灭从边缘逼近的无人机，波次递增，难度上升。\par' +
    '\par' +

    '\pard\b\fs24\cf3 8. 软件设置\cf0\b0\fs20\par' +
    '支持跟随系统/深色/浅色主题，亚克力毛玻璃效果。\par' +
    '侧边栏自动收回选项，AI 服务商可选Ollama/DeepSeek。\par' +
    '\par' +

    '\pard\qc\fs16\cf0 点击「下一步」继续安装\par' +
    '}';
end;

// 卸载时清理设置文件
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    if FileExists(ExpandConstant('{app}\settings.ini')) then
      DeleteFile(ExpandConstant('{app}\settings.ini'));
  end;
end;
