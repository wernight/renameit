@echo off
setlocal enabledelayedexpansion
set script_dir=%~dp0
set echoc=%script_dir%..\util\echoc.exe

echo Cleaning up the test folders...
if exist "%~dp0tmp" (
    rmdir /s /q "%~dp0tmp" || (
        "%echoc%" 0 12 Command failed: rmdir "%~dp0tmp"
        exit /B 1
    )
)
echo.
exit /B 0
