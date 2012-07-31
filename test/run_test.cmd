@echo off
set IN_RUN_ALL_TESTS=1
setlocal enabledelayedexpansion

if not exist RenameIt.exe (
	echo RenameIt.exe is missing.
	echo Please copy it in this folder to test it.
	echo.
	pause
	exit /B 1
)

set failed_tests=0

echo ==========================================
echo TEST COMMAND-LINE
echo ==========================================
pushd "%~dp0test Command-line"
call run_test.cmd
set /A failed_tests=%failed_tests%+%ERRORLEVEL%
popd
popd
echo.

echo ==========================================
echo TEST FILTERS
echo ==========================================
pushd "%~dp0test Filters"
call run_test.cmd
set /A failed_tests=%failed_tests%+%ERRORLEVEL%
popd
echo.

pause
exit /B %failed_tests%
