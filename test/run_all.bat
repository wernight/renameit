@echo off

if exist RenameIt.exe goto ready
	echo RenameIt.exe is missing.
	echo Please copy it in this folder to test it.
	echo.
	pause
	exit 1
:ready

echo ==========================================
echo TEST COMMAND-LINE
echo ==========================================
cd "test Command-line"
call run.bat
if %errorlevel%==0 goto end
cd ..
echo.

echo ==========================================
echo TEST FILTERS
echo ==========================================
cd "test Filters"
call run.bat
cd ..
echo.

:end
pause
