@echo off
chcp 65001 >nul
title BasicUI Framework Installer
echo ========================================
echo   BasicUI Framework One-Click Installer
echo ========================================
echo.

:: Check for PowerShell
powershell -Command "Get-Host" >nul 2>&1
if %errorlevel% neq 0 (
    echo [X] PowerShell not found. This installer requires PowerShell 5.1 or later.
    pause
    exit /b 1
)

:: Parse arguments
set COMPILER=MSVC
if /I "%1"=="mingw" set COMPILER=MinGW
if /I "%1"=="msvc" set COMPILER=MSVC

echo Compiler: %COMPILER%
echo.

:: Run the PowerShell installer
powershell -ExecutionPolicy Bypass -File "%~dp0install.ps1" -Compiler %COMPILER%

if %errorlevel% neq 0 (
    echo.
    echo [X] Installation failed. See errors above.
    pause
    exit /b 1
)

echo.
pause
