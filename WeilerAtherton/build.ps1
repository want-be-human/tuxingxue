# 编译脚本 (使用 MinGW)
# 运行此脚本前请确保已安装 MinGW 并添加到系统 PATH

g++ -o WeilerAtherton.exe main.cpp WeilerAtherton.cpp -lgdi32 -luser32 -mwindows -std=c++11 -O2 -municode

if ($LASTEXITCODE -eq 0) {
    Write-Host "编译成功! 可执行文件: WeilerAtherton.exe" -ForegroundColor Green
    Write-Host "运行程序请执行: .\WeilerAtherton.exe" -ForegroundColor Cyan
} else {
    Write-Host "编译失败,请检查错误信息" -ForegroundColor Red
}
