@echo off
setlocal
cd /d "%~dp0"
where gcc >NUL 2>&1
if errorlevel 1 (
  echo ERROR: MinGW-w64 GCC was not found in PATH.
  echo Install MSYS2 UCRT64 and add C:\msys64\ucrt64\bin to PATH.
  exit /b 1
)
if not exist dist mkdir dist
gcc -municode -O2 -std=c11 -Wall -Wextra -Wpedantic -Iinclude src\main.c src\buffer.c src\cmd.c src\fileio.c src\utf8.c src\cvnss_parse.c src\cvnss_decode.c src\cvnss_encode.c -o dist\cved.exe
if errorlevel 1 exit /b 1
echo Built dist\cved.exe
"dist\cved.exe" --version
endlocal
