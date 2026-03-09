@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

REM WoW Simbot Quick - Release Build Script
REM Requires: Visual Studio 2022, MSBuild

echo ==========================================
echo WoW Simbot Quick - Release Build Script
echo ==========================================
echo.

REM Configuration
set SOLUTION_NAME=WoWSimbotQuick.sln
set CONFIGURATION=Release
set PLATFORM=x64
set OUTPUT_DIR=bin\Release
set DIST_DIR=dist
set VERSION=1.0.0

REM Check prerequisites
where msbuild >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] MSBuild not found. Please run this script from Developer Command Prompt.
    exit /b 1
)

REM Clean previous builds
echo [1/6] Cleaning previous builds...
if exist %OUTPUT_DIR% (
    rmdir /s /q %OUTPUT_DIR%
)
if exist %DIST_DIR% (
    rmdir /s /q %DIST_DIR%
)
if exist obj (
    rmdir /s /q obj
)

REM Restore NuGet packages (if any)
echo [2/6] Restoring packages...
REM nuget restore %SOLUTION_NAME%

REM Build solution
echo [3/6] Building solution (%CONFIGURATION%|%PLATFORM%)...
msbuild %SOLUTION_NAME% /p:Configuration=%CONFIGURATION% /p:Platform=%PLATFORM% /m /v:minimal
if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    exit /b 1
)

REM Check if output exists
if not exist %OUTPUT_DIR%\WoWSimbotQuick.exe (
    echo [ERROR] Build output not found!
    exit /b 1
)

REM Create distribution directory
echo [4/6] Preparing distribution package...
mkdir %DIST_DIR%
mkdir %DIST_DIR%\Examples

REM Copy files
copy %OUTPUT_DIR%\WoWSimbotQuick.exe %DIST_DIR%\
copy %OUTPUT_DIR%\*.dll %DIST_DIR%\ 2>nul

REM Copy documentation
copy README.md %DIST_DIR%\
copy LICENSE.txt %DIST_DIR%\
copy CHANGELOG.md %DIST_DIR%\

REM Copy examples
copy examples\*.simc %DIST_DIR%\Examples\ 2>nul

echo [5/6] Verifying distribution package...
dir %DIST_DIR% /b

REM Create archive
echo [6/6] Creating archive...
if exist "C:\Program Files\7-Zip\7z.exe" (
    "C:\Program Files\7-Zip\7z.exe" a -tzip "WoWSimbotQuick_v%VERSION%_portable.zip" "%DIST_DIR%\*"
) else (
    powershell Compress-Archive -Path "%DIST_DIR%\*" -DestinationPath "WoWSimbotQuick_v%VERSION%_portable.zip" -Force
)

echo.
echo ==========================================
echo Build completed successfully!
echo.
echo Output files:
echo   - %OUTPUT_DIR%\WoWSimbotQuick.exe
echo   - WoWSimbotQuick_v%VERSION%_portable.zip
echo ==========================================

endlocal
