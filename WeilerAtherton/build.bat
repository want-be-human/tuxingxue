@echo off
REM 编译脚本 (使用 MinGW)
REM 运行此脚本前请确保已安装 MinGW 并添加到系统 PATH

echo 正在编译 Weiler-Atherton 多边形裁剪程序...

g++ -o WeilerAtherton.exe main.cpp WeilerAtherton.cpp -lgdi32 -luser32 -mwindows -std=c++11 -O2 -municode

if %ERRORLEVEL% EQU 0 (
    echo.
    echo 编译成功! 可执行文件: WeilerAtherton.exe
    echo 运行程序请执行: WeilerAtherton.exe
    echo.
) else (
    echo.
    echo 编译失败,请检查错误信息
    echo.
)

pause
