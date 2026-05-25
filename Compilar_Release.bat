@echo off
title Compilando MuMain [RELEASE]
cls
echo ===================================================
echo   Compilando MuMain en modo RELEASE (Optimizada)
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
echo   Ejecutando Compilacion [RELEASE]...
echo ===================================================
powershell -NoProfile -ExecutionPolicy Bypass -Command "Import-Module 'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\Microsoft.VisualStudio.DevShell.dll'; Enter-VsDevShell -VsInstallPath 'C:\Program Files\Microsoft Visual Studio\2022\Professional' -Arch x86 -SkipAutomaticLocation; & 'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build --preset windows-x86-release"

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
echo   Ubicacion: out/build/windows-x86/src/Release/Main.exe
echo ===================================================
echo.
pause
