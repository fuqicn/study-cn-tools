@echo off
chcp 65001 >nul
echo ==========================================
echo 国防安全科普教育软件 - 静态编译脚本
echo ==========================================
echo.
echo 注意: 静态编译需要Qt静态库版本
echo.

:: 检查Qt静态版本
if not defined QT_STATIC_PATH (
    set QT_STATIC_PATH=C:\Qt\Static\mingw64
)

if not exist "%QT_STATIC_PATH%\bin\qmake.exe" (
    echo 错误: 未找到Qt静态版本
echo 请设置QT_STATIC_PATH环境变量指向Qt静态版本目录
echo 例如: set QT_STATIC_PATH=C:\Qt\Static\mingw64
    pause
    exit /b 1
)

set PATH=%QT_STATIC_PATH%\bin;%PATH%

:: 创建构建目录
if not exist build-static mkdir build-static
cd build-static

echo 正在清理旧构建...
if exist Makefile mingw32-make clean >nul 2>nul

echo.
echo 正在生成Makefile(静态链接)...
qmake ..\defense-edu.pro CONFIG+=static
if %errorlevel% neq 0 (
    echo 错误: qmake失败
    pause
    exit /b 1
)

echo.
echo 正在静态编译(这可能需要几分钟)...
mingw32-make -j4
if %errorlevel% neq 0 (
    echo 错误: 编译失败
    pause
    exit /b 1
)

echo.
echo ==========================================
echo 静态编译成功!
echo 可执行文件: build-static\defense-edu.exe
echo 此exe文件可以独立运行，无需Qt DLL
echo ==========================================
echo.

:: 复制到上级目录
copy defense-edu.exe ..\defense-edu-static.exe >nul

cd ..

pause
