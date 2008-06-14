@echo off

if exist RenameIt.exe goto ready
	echo RenameIt.exe is missing.
	echo Please copy it in this folder to test it.
	echo.
	pause
	exit 1
:ready

set IN_RUN_ALL=1

echo ==========================================
echo TEST COMMAND-LINE
echo ==========================================
cd "test Command-line"
call run.cmd
if errorlevel 1 goto end
cd ..
echo.

echo ==========================================
echo TEST FILTERS
echo ==========================================
cd "test Filters"
call run_all.cmd
cd ..
echo.

:end
pause
