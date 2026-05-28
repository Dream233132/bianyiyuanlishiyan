@echo off
chcp 65001 >nul
cd /d "%~dp0"
g++ -std=c++11 -Wall lexer.cpp -o lexer.exe
if %errorlevel% equ 0 (
    echo 编译成功！
) else (
    echo 编译失败！
)
pause
