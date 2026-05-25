@echo off
title Compilando MuMain [DEBUG]
cls
echo ===================================================
echo   Compilando MuMain en modo DEBUG (Desarrollo)
echo ===================================================
echo.

:: Buscar si el cliente esta corriendo para evitar bloqueos
tasklist /FI "IMAGENAME eq Main.exe" 2>NUL | find /I /N "Main.exe">NUL
if "%ERRORLEVEL%"=="0" (
    echo [ADVERTENCIA] El cliente Main.exe esta en ejecucion.
    echo Cierra el juego antes de continuar para evitar errores de bloqueo de archivos.
    echo.
    pause
)

cls
echo ===================================================
echo   Selecciona el Preset de Compilacion:
echo ===================================================
echo [1] Debug Estandar (windows-x86-debug)
echo [2] Debug con Editor Integrado (windows-x86-mueditor-debug)
echo.
set /p opcion="Ingresa tu opcion (1 o 2): "

if "%opcion%"=="1" (
    set PRESET=windows-x86-debug
    set TITLE=Debug Estandar
) else if "%opcion%"=="2" (
    set PRESET=windows-x86-mueditor-debug
    set TITLE=Debug con Editor
) else (
    echo.
    echo [ERROR] Opcion invalida. Saliendo...
    echo.
    pause
    exit /b 1
)

cls
echo ===================================================
echo   Ejecutando Compilacion [%TITLE%]...
echo ===================================================
powershell -NoProfile -ExecutionPolicy Bypass -Command "Import-Module 'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\Microsoft.VisualStudio.DevShell.dll'; Enter-VsDevShell -VsInstallPath 'C:\Program Files\Microsoft Visual Studio\2022\Professional' -Arch x86 -SkipAutomaticLocation; & 'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build --preset %PRESET%"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] La compilacion ha fallado. Revisa los mensajes de arriba.
    echo.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ===================================================
echo   [OK] Compilacion completada con exito!
echo   Ubicacion: out/build/windows-x86/src/Debug/Main.exe
echo ===================================================
echo.
pause
