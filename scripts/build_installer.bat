@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

REM WoW Simbot Quick - Installer Build Script
REM Requires: Inno Setup 6

echo ==========================================
echo WoW Simbot Quick - Installer Build Script
echo ==========================================
echo.

REM Configuration
set ISS_FILE=setup\WoWSimbotQuick.iss
set ISCC_PATH="C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
set VERSION=1.0.0

REM Find ISCC
if not exist %ISCC_PATH% (
    set ISCC_PATH="C:\Program Files\Inno Setup 6\ISCC.exe"
)

if not exist %ISCC_PATH% (
    echo [ERROR] Inno Setup not found. Please install Inno Setup 6.
    echo Download from: https://jrsoftware.org/isdl.php
    exit /b 1
)

REM Check if release build exists
if not exist bin\Release\WoWSimbotQuick.exe (
    echo [WARNING] Release build not found. Running build script first...
    call scripts\build_release.bat
    if %errorlevel% neq 0 (
        echo [ERROR] Build failed!
        exit /b 1
    )
)

REM Create installer directory
if not exist installer mkdir installer

REM Build installer
echo Building installer...
%ISCC_PATH% /Q /Oinstaller %ISS_FILE%
if %errorlevel% neq 0 (
    echo [ERROR] Installer build failed!
    exit /b 1
)

echo.
echo ==========================================
echo Installer created successfully!
echo.
echo Output: installer\WoWSimbotQuick_Setup_%VERSION%.exe
echo ==========================================

endlocal
