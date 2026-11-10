@echo off
setlocal
cd /d "%~dp0"
if not exist dist\cved.exe call build-windows.cmd
if not exist dist\cved.exe exit /b 1
chcp 65001 >NUL
"dist\cved.exe" demo.txt
endlocal
