@echo off
setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%
cd /d "%SCRIPT_DIR%"

set CLI=%SCRIPT_DIR%\arduino-cli.exe
set CONFIG=arduino-cli.yaml
set FQBN=arduino:renesas_uno:unor4wifi
set BASE_FLAGS=-I src -I include
set LIBS=--library lib/arduino_s4sMainBoard --library lib/arduino_k210 --library lib/music_i2sPlayer --library lib/udcheck --library lib/zs_tools

set CMD=%1
if "%CMD%"=="" goto usage
shift

set PORT=
set BAUDRATE=
set EXTRA_DEFS=
set VERBOSE=

:parse_args
if "%1"=="" goto end_parse
if /i "%1"=="-v" (
    set VERBOSE=1
    shift
    goto parse_args
)
if /i "%1"=="-d" (
    if not "%2"=="" set "EXTRA_DEFS=!EXTRA_DEFS! -D%2"
    shift
    shift
    goto parse_args
)
if not defined PORT (
    set PORT=%1
    shift
    goto parse_args
)
set BAUDRATE=%1
shift
goto parse_args
:end_parse

if /i "%CMD%"=="init" goto init
if /i "%CMD%"=="clear" goto clear
if /i "%CMD%"=="build" goto build
if /i "%CMD%"=="burn" goto burn

:usage
echo ====================================
echo   S4S Hat - build.bat
echo ====================================
echo.
echo Usage:
echo   build.bat init                  Install core ^& libraries
echo   build.bat clear                 Clear build cache
echo   build.bat build [-v] [-d MACRO...]   Compile (use -v to show compiled files)
echo   build.bat burn PORT [BAUD] [-v] [-d MACRO...]
echo.
echo Examples:
echo   build.bat build
echo   build.bat build -v
echo   build.bat build -d NOT_CUSTOM_BUILD
echo   build.bat build -v -d NOT_CUSTOM_BUILD
echo   build.bat burn COM99
echo   build.bat burn COM99 115200
echo   build.bat burn COM99 -d NOT_CUSTOM_BUILD
exit /b 1

rem ======================== init ========================
:init
echo ====================================
echo   Init: Install core
echo ====================================
"%CLI%" --config-file "%CONFIG%" core update-index
if errorlevel 1 goto :eof
"%CLI%" --config-file "%CONFIG%" core install arduino:renesas_uno
if errorlevel 1 goto :eof

echo.
echo ====================================
echo   Init: Install libraries
echo ====================================
"%CLI%" --config-file "%CONFIG%" lib install "Adafruit BusIO"
"%CLI%" --config-file "%CONFIG%" lib install "Adafruit GFX Library"
"%CLI%" --config-file "%CONFIG%" lib install "Adafruit QMC5883P Library"
"%CLI%" --config-file "%CONFIG%" lib install "Adafruit SH110X"
"%CLI%" --config-file "%CONFIG%" lib install "Seeed Arduino LSM6DS3"

echo.
echo ====================================
echo   Init: File management
echo ====================================
if not exist "src.ino" (
    echo   Creating src.ino ...
    (
        echo #include "prepare.h"
        echo.
        echo void app_setup(void^)
        echo {
        echo }
        echo.
        echo void app_loop(void^)
        echo {
        echo }
    ) > "src.ino"
)
if exist "src\main.cpp" (
    if not exist "src\main.cpp_" (
        echo   Renaming src\main.cpp to src\main.cpp_ ...
        move "src\main.cpp" "src\main.cpp_"
    )
)
set EXAMPLE_DIR=lib\zs_tools\src\component\cc\example
if exist "%EXAMPLE_DIR%\" (
    dir /b "%EXAMPLE_DIR%\*.c" >nul 2>&1
    if !errorlevel! equ 0 (
        echo   Renaming .c files in example/ to avoid compilation ...
        ren "%EXAMPLE_DIR%\*.c" *.c_
    )
)

echo.
echo ========== Init complete ==========
exit /b 0

rem ======================== clear ========================
:clear
echo ========== Clear cache ==========
if exist arduino_data\staging (
    rmdir /s /q arduino_data\staging
    echo   Cache cleared
) else (
    echo   No cache found
)
exit /b 0

rem ======================== build ========================
:build
echo ========== File management ==========
if exist "src\main.cpp" (
    if not exist "src\main.cpp_" (
        echo   Renaming src\main.cpp to src\main.cpp_ ...
        move "src\main.cpp" "src\main.cpp_"
    )
)
set EXAMPLE_DIR=lib\zs_tools\src\component\cc\example
if exist "%EXAMPLE_DIR%\" (
    dir /b "%EXAMPLE_DIR%\*.c" >nul 2>&1
    if !errorlevel! equ 0 (
        echo   Renaming .c files in example/ to avoid compilation ...
        ren "%EXAMPLE_DIR%\*.c" *.c_
    )
)

echo.
echo ========== Compiling ... ==========
set BUILD_FLAGS=%BASE_FLAGS% %EXTRA_DEFS%
if defined VERBOSE (
    "%CLI%" --config-file "%CONFIG%" compile --fqbn %FQBN% --build-property "build.extra_flags=%BUILD_FLAGS%" %LIBS% "%SCRIPT_DIR%" -v
    set CLI_EXIT=!errorlevel!
) else (
    "%CLI%" --config-file "%CONFIG%" compile --fqbn %FQBN% --build-property "build.extra_flags=%BUILD_FLAGS%" %LIBS% "%SCRIPT_DIR%"
    set CLI_EXIT=!errorlevel!
)

echo.
if !CLI_EXIT! neq 0 (
    echo ========== BUILD FAILED ==========
) else (
    echo ========== BUILD SUCCESSFUL ==========
)
exit /b 0

rem ======================== burn ========================
:burn
if "%PORT%"=="" (
    echo [ERROR] PORT is required for burn.
    echo.
    goto usage
)

echo ========== File management ==========
if exist "src\main.cpp" (
    if not exist "src\main.cpp_" (
        echo   Renaming src\main.cpp to src\main.cpp_ ...
        move "src\main.cpp" "src\main.cpp_"
    )
)
set EXAMPLE_DIR=lib\zs_tools\src\component\cc\example
if exist "%EXAMPLE_DIR%\" (
    dir /b "%EXAMPLE_DIR%\*.c" >nul 2>&1
    if !errorlevel! equ 0 (
        echo   Renaming .c files in example/ to avoid compilation ...
        ren "%EXAMPLE_DIR%\*.c" *.c_
    )
)

echo.
echo ========== Compiling ... ==========
set BUILD_FLAGS=%BASE_FLAGS% %EXTRA_DEFS%
if defined VERBOSE (
    "%CLI%" --config-file "%CONFIG%" compile --fqbn %FQBN% --build-property "build.extra_flags=%BUILD_FLAGS%" %LIBS% "%SCRIPT_DIR%" -v
    set CLI_EXIT=!errorlevel!
) else (
    "%CLI%" --config-file "%CONFIG%" compile --fqbn %FQBN% --build-property "build.extra_flags=%BUILD_FLAGS%" %LIBS% "%SCRIPT_DIR%"
    set CLI_EXIT=!errorlevel!
)

if !CLI_EXIT! neq 0 (
    echo ========== BUILD FAILED, aborting upload ==========
    exit /b 1
)

echo.
echo ========== Uploading to %PORT% ... ==========
"%CLI%" --config-file "%CONFIG%" upload ^
    --fqbn %FQBN% ^
    --port %PORT%

echo.
if errorlevel 1 (
    echo ========== UPLOAD FAILED ==========
) else (
    echo ========== UPLOAD SUCCESSFUL ==========
)
exit /b 0
