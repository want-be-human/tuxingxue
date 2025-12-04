@echo off
setlocal

REM Switch to this script's directory so relative paths are reliable
cd /d "%~dp0"

echo ====================================
echo MinGW g++ build
echo ====================================
echo(

if not exist build (
    echo Creating build directory...
    mkdir build
)

echo Compiling...
echo(

g++ -std=c++17 -DUNICODE -D_UNICODE -Isrc ^
    src/MainWindow.cpp src/Shape.cpp src/Canvas.cpp src/DrawingAlgorithm.cpp ^
    -o build/GraphicsApp.exe ^
    -luser32 -lgdi32 -lcomctl32 -mwindows -static

if errorlevel 1 goto :fail

echo(
echo Build SUCCESS
echo Output: build\GraphicsApp.exe
echo(
set /p RUN_APP=Run now? [Y/N]: 
if /i "%RUN_APP%"=="Y" start "" ".\build\GraphicsApp.exe"
goto :end

:fail
echo(
echo Build FAILED
echo Ensure MinGW is installed and on PATH (g++, windres, etc.)

:end
endlocal
pause
