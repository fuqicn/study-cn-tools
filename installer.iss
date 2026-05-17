; 国防安全科普教育软件 - Inno Setup 安装脚本
; 制作者：傅琪

#define AppName "国防安全科普教育软件"
#define AppVersion "3.2.3"
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
DefaultDirName={userpf}\{#AppName}
PrivilegesRequired=lowest
DisableDirPage=no
UsedUserAreasWarning=no
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
CloseApplications=yes

[Messages]
SetupAppTitle=安装 - {#AppName}
SetupWindowTitle=安装 - {#AppName}
WelcomeLabel1=欢迎使用 {#AppName} 安装向导
WelcomeLabel2=这将安装 {#AppName} 到您的计算机。%n%n本软件集成了国防装备展示、时间轴、AI 智能问答、AI 答题、知识问答、知识答题、激光防御模拟等功能。%n%n制作者：傅琪%n%n声明：Qt 不用于任何公司和商业用途。%n%n建议在继续之前关闭所有其他应用程序。%n%n版本 3.2.3 改进：修复25+项Bug，优化 Markdown 渲染、信号管理、防抖保存等。
SelectDirLabel3=安装程序将把 {#AppName} 安装到以下文件夹。
SelectDirBrowseLabel=如需安装到其他文件夹，请点击「浏览」。
ReadyLabel1=安装程序已准备好将 {#AppName} 安装到您的计算机。
ReadyLabel2a=点击「安装」开始安装，或点击「上一步」修改设置。
InstallingLabel=正在安装 {#AppName}，请稍候...
FinishedHeadingLabel={#AppName} 安装完成
FinishedLabelNoIcons={#AppName} 已成功安装到您的计算机。
FinishedLabel={#AppName} 已成功安装到您的计算机。
ClickFinish=点击「完成」退出安装向导。
ClickNext=点击「下一步」继续。
LicenseLabel3=请在安装之前阅读以下声明。如果您接受该声明，请点击「我同意」继续。
LicenseAccepted=我同意此声明(&A)
LicenseNotAccepted=我不同意此声明
WizardLicense=声明
WizardSelectDir=选择安装目录
WizardReady=准备安装
WizardInstalling=正在安装
WizardSelectProgramGroup=选择开始菜单文件夹
ButtonNext=下一步(&N) >
ButtonInstall=安装(&I)
ButtonBack=< 上一步(&B)
ButtonCancel=取消
ButtonFinish=完成(&F)
ButtonBrowse=浏览(&R)...
ExitSetupMessage=您确定要退出安装程序吗？%n%n安装尚未完成。
AboutSetupMenuItem=关于安装程序(&A)
AboutSetupMessage=安装程序版本: %1%n%n版权所有(C) 2026 傅琪%n%n本安装程序使用 Inno Setup 制作。%n%nhttps://www.innosetup.com/
BrowseDialogTitle=浏览文件夹
BrowseDialogLabel=请选择目标文件夹：
SetupLdrStartupMessage=安装程序将安装 {#AppName}。是否继续？
UninstalledAll={#AppName} 已成功从您的计算机中删除。
StatusCreateDirs=正在创建目录...
StatusCreateIcons=正在创建快捷方式...
StatusCreateIniEntries=正在创建 INI 条目...
StatusCreateRegistryEntries=正在创建注册表条目...
StatusRegisterFiles=正在注册文件...
StatusSavingUninstall=正在保存卸载信息...
StatusRunProgram=正在完成安装...
StatusRollback=正在回滚更改...
StatusExtractFiles=正在提取文件...
SelectDirDesc=选择 {#AppName} 的安装目录
ReadyLabel2b=安装类型：
ReadyMemoUserInfo=用户信息：
ReadyMemoDir=安装目录：
ReadyMemoGroup=开始菜单文件夹：
ReadyMemoTasks=附加任务：
FinishedRestartLabel=是否立即重启计算机？
FinishedRestartMessage=点击「完成」以重启计算机。%n%n请先保存您的工作，然后继续。
WinVersionTooLowError=此安装程序需要 %1 或更高版本。
CannotInstallToNetworkDrive=无法安装到网络驱动器。
ApplicationsFound=以下应用程序正在使用需要更新的文件。建议您关闭这些应用程序以允许安装程序更新它们。
ApplicationsFound2=请关闭以下应用程序：%n%n%1
ChangeDiskTitle=更改磁盘
UninstallStatusLabel=正在从您的计算机中删除 {#AppName}，请稍候...
ConfirmUninstall=您确定要完全删除 {#AppName} 及其所有组件吗？
UninstallNotFound=未找到卸载日志文件。%n%n是否仍然删除程序组和图标？
NoUninstallWarning=卸载程序可能无法完全删除程序的所有文件。%n%n是否继续？
SelectStartMenuFolderLabel3=安装程序将在以下开始菜单文件夹中创建程序快捷方式。
SelectStartMenuFolderDesc=请选择开始菜单文件夹以放置程序快捷方式。
SelectStartMenuFolderBrowseLabel=如需选择其他文件夹，请点击「浏览」。
DiskSpaceMBLabel=至少需要 [mb] MB 的可用磁盘空间。
DiskSpaceGBLabel=至少需要 [gb] GB 的可用磁盘空间。
DiskSpaceWarning=安装程序需要至少 %1 KB 的可用空间，但所选驱动器只有 %2 KB。%n%n是否仍然继续？
DiskSpaceWarningTitle=磁盘空间不足
ExtractingLabel=正在提取文件...
SelectTasksDesc=需要执行哪些附加任务？
SelectTasksLabel2=请选择安装程序在安装 {#AppName} 时需要执行的附加任务，然后点击「下一步」。
WizardPreparing=准备安装
WizardSelectTasks=选择附加任务
PreparingDesc=安装程序正在准备将 {#AppName} 安装到您的计算机。
SelectDirectoryLabel=请指定下一张磁盘的位置。
StatusClosingApplications=正在关闭应用程序...
StatusRestartingApplications=正在重新启动应用程序...
InfoBeforeLabel=请先阅读以下重要信息，然后再继续。
InfoAfterLabel=请在继续之前阅读以下重要信息。
DirDoesntExist=文件夹：%n%n%1%n%n不存在。是否创建该文件夹？
DirDoesntExistTitle=文件夹不存在
DirExists=文件夹：%n%n%1%n%n已存在。是否仍然安装到该文件夹？
DirExistsTitle=文件夹已存在
NoProgramGroupCheck2=不创建开始菜单文件夹
SetupAborted=安装未完成。%n%n请纠正问题后重新运行安装程序。
UninstallAppFullTitle=卸载 {#AppName}
UninstallAppRunningError=卸载程序检测到 {#AppName} 正在运行。%n%n请关闭所有实例，然后点击「确定」继续，或点击「取消」退出。
ButtonNewFolder=新建文件夹(&M)

[CustomMessages]
DesktopIcon=创建桌面快捷方式
RunAfterInstall=安装完成后运行 {#AppName}

[Tasks]
Name: "desktopicon"; Description: "{cm:DesktopIcon}"; GroupDescription: "附加快捷方式:"
Name: "runafterinstall"; Description: "{cm:RunAfterInstall}"; GroupDescription: "其他选项:"

[Files]
Source: "..\appfolder\defense-edu.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\Qt6Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\Qt6Svg.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\libgcc_s_seh-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\D3Dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\appfolder\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs
Source: "..\appfolder\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs
Source: "..\appfolder\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs
Source: "..\appfolder\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs
Source: "..\appfolder\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs
Source: "..\appfolder\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs
Source: "..\appfolder\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{group}\卸载 {#AppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:RunAfterInstall}"; Flags: nowait postinstall skipifsilent; Tasks: runafterinstall
