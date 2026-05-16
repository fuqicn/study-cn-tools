@echo off
chcp 65001 >nul
echo ==========================================
echo 国防安全科普教育软件 - 编译部署脚本
echo ==========================================
echo.

:: 检查Qt环境
where qmake >nul 2>nul
if %errorlevel% neq 0 (
    echo 错误: 未找到qmake，请确保Qt已安装并添加到PATH
    echo 请设置Qt环境变量，例如:
    echo   set PATH=C:\Qt\6.x.x\mingw_64\bin;%%PATH%%
    pause
    exit /b 1
)

:: 创建构建目录
if not exist build mkdir build
cd build

echo 正在清理旧构建...
if exist Makefile mingw32-make clean >nul 2>nul
if exist defense-edu.exe del defense-edu.exe

echo.
echo 正在生成Makefile...
qmake ..\defense-edu.pro
if %errorlevel% neq 0 (
    echo 错误: qmake失败
    cd ..
    pause
    exit /b 1
)

echo.
echo 正在编译...
mingw32-make -j4
if %errorlevel% neq 0 (
    echo 错误: 编译失败
    cd ..
    pause
    exit /b 1
)

echo.
echo ==========================================
echo 编译成功!
echo ==========================================
echo.

cd ..

:: 部署到输出目录
set OUTPUT_DIR=..\国防科普教育软件

echo 正在部署到 %OUTPUT_DIR%...
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

:: 复制exe
copy build\defense-edu.exe "%OUTPUT_DIR%\" >nul

:: 运行windeployqt部署依赖
echo 正在运行 windeployqt...
windeployqt --release --no-translations "%OUTPUT_DIR%\defense-edu.exe"
if %errorlevel% neq 0 (
    echo 警告: windeployqt执行失败，可能缺少DLL
)

echo.
echo ==========================================
echo 部署完成!
echo 输出目录: %OUTPUT_DIR%
echo ==========================================
echo.

echo 是否运行程序？(Y/N)
set /p choice=
if /i "%choice%"=="Y" (
    start "" "%OUTPUT_DIR%\defense-edu.exe"
)

pause
