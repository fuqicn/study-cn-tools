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
DefaultDirName={code:GetDefaultDirName}
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=commandline
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
; --- 全面覆盖英文消息为中文 ---
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
  InstallScopePage: TInputOptionWizardPage;

const
  BCM_SETSHIELD = $0000160C;

function SendMessage(hWnd: Integer; Msg: Integer; wParam: Integer; lParam: Integer): Integer;
  external 'SendMessageW@user32.dll stdcall';

function IsAdminUser: Boolean;
begin
  Result := IsAdmin;
end;

// 判断当前是否为"所有用户"安装模式
function IsAllUsersMode: Boolean;
begin
  // 优先级：命令行参数 > 安装范围页的选择
  if ExpandConstant('{param:ALLUSERS|}') <> '' then
    Result := True
  else if ExpandConstant('{param:CURRENTUSER|}') <> '' then
    Result := False
  else
    Result := InstallScopePage.Values[0];
end;

// 动态安装目录：所有用户 → Program Files，当前用户 → 用户 Programs
function GetDefaultDirName(Param: String): String;
begin
  if IsAllUsersMode then
    Result := ExpandConstant('{autopf}\{#AppName}')
  else
    Result := ExpandConstant('{userpf}\{#AppName}');
end;

// 设置存储目录
function GetSettingsDir(Param: String): String;
begin
  if IsAllUsersMode then
    Result := ExpandConstant('{commonappdata}\DefenseEdu')
  else
    Result := ExpandConstant('{localappdata}\DefenseEdu');
end;

// 更新"下一步"按钮的盾牌图标
procedure UpdateShield;
begin
  // 选择「所有用户」且当前不是管理员 → 显示盾牌
  if InstallScopePage.Values[0] and not IsAdminUser then
    SendMessage(WizardForm.NextButton.Handle, BCM_SETSHIELD, 0, 1)
  else
    SendMessage(WizardForm.NextButton.Handle, BCM_SETSHIELD, 0, 0);
end;

procedure InitializeWizard;
begin
  // 安装范围选择页（在欢迎页之后、许可协议页之前）
  InstallScopePage := CreateInputOptionPage(
    wpWelcome,
    '选择安装范围',
    '请选择安装范围',
    '选择「所有用户」需要管理员权限。',
    True,   // exclusive (radio buttons)
    False   // not list boxes
  );
  InstallScopePage.Add('安装给所有用户（需要管理员权限）');
  InstallScopePage.Add('仅安装给当前用户');
  InstallScopePage.Values[0] := IsAdminUser;
  InstallScopePage.Values[1] := not IsAdminUser;

  // 非管理员默认仅当前用户（选择项保持可用，用户自行决定）
  if not IsAdminUser then
  begin
    InstallScopePage.Values[0] := False;
    InstallScopePage.Values[1] := True;
  end;

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

    '\pard\qc\b\fs28\cf1 国防安全科普教育软件 v3.2.3\cf0\b0\fs22\par' +
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

// 如果是通过提权重启的（命令行带 /ALLUSERS 或 /CURRENTUSER），跳过安装范围选择页
function ShouldSkipPage(PageID: Integer): Boolean;
begin
  if PageID = InstallScopePage.ID then
    Result := (ExpandConstant('{param:ALLUSERS|}') <> '') or
              (ExpandConstant('{param:CURRENTUSER|}') <> '')
  else
    Result := False;
end;

// 点击"下一步"时的处理
function NextButtonClick(CurPageID: Integer): Boolean;
var
  ErrorCode: Integer;
begin
  Result := True;

  if CurPageID = InstallScopePage.ID then
  begin
    if InstallScopePage.Values[0] and not IsAdminUser then
    begin
      // 用户选择了「所有用户」但当前不是管理员
      // 以管理员身份重启安装程序，传入 /ALLUSERS 标记
      if ShellExec('runas', ExpandConstant('{srcexe}'), '/ALLUSERS /NORESTART',
                   '', SW_SHOW, ewNoWait, ErrorCode) then
      begin
        Result := False;  // 关闭当前（非管理员）安装程序
      end
      else
      begin
        // 提权失败（用户取消UAC等）
        if MsgBox('无法以管理员身份运行安装程序。'#13#13 +
                  '要继续安装给所有用户，请以管理员身份重新运行本安装程序。'#13#13 +
                  '是否改为仅安装给当前用户？',
                  mbError, MB_YESNO) = IDYES then
        begin
          InstallScopePage.Values[0] := False;
          InstallScopePage.Values[1] := True;
          UpdateShield;
          Result := True;
        end
        else
          Result := False;  // 取消安装
      end;
    end;
  end;
end;

// 页面切换时更新盾牌
procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = InstallScopePage.ID then
    UpdateShield;
end;

// 安装后处理
procedure CurStepChanged(CurStep: TSetupStep);
var
  ScopeFile, SettingsDir: String;
begin
  if CurStep = ssPostInstall then
  begin
    // 写入安装范围标记文件
    ScopeFile := ExpandConstant('{app}\.install-scope');
    if IsAllUsersMode then
      SaveStringToFile(ScopeFile, 'all', False)
    else
      SaveStringToFile(ScopeFile, 'user', False);

    // 确保设置目录存在
    SettingsDir := GetSettingsDir('');
    if not DirExists(SettingsDir) then
      ForceDirectories(SettingsDir);
  end;
end;

// 卸载时清理设置文件
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  AppDataPath, ProgDataPath: String;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    // 尝试清理两个可能的设置目录
    AppDataPath := ExpandConstant('{localappdata}\DefenseEdu');
    if DirExists(AppDataPath) then
    begin
      DeleteFile(AppDataPath + '\settings.ini');
      RemoveDir(AppDataPath);
    end;
    ProgDataPath := ExpandConstant('{commonappdata}\DefenseEdu');
    if DirExists(ProgDataPath) then
    begin
      DeleteFile(ProgDataPath + '\settings.ini');
      RemoveDir(ProgDataPath);
    end;
  end;
end;
