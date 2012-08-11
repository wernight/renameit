@echo off
set ORIGINAL_DIR=%CD%
set TEST_DIR=%~p0
set TEST_EXE_NAME=CompFold.exe

cd "%TEST_DIR%"

:: Check that the app to test is there
if exist "%TEST_EXE_NAME%" goto app_found
	echo *** `%TEST_EXE_NAME%` not found in this folder.
	echo     Please copy it here first, so it can be tested.
	pause
	exit /B 2
:app_found

:: Extract test files (if necessary)
if exist f1 if exist f2 goto test_files_extracted
	extract_test_files.exe -y
:test_files_extracted

:: Assert f1==f1
"%TEST_EXE_NAME%" f1 f1
if errorlevel 1 goto test_failed

:: Assert f2==f2
"%TEST_EXE_NAME%" f2 f2
if errorlevel 1 goto test_failed

:: Assert f1!=f2 by 8 files
"%TEST_EXE_NAME%" f1 f2
if errorlevel 9 goto test_failed
if not errorlevel 8 goto test_failed

color 2
echo [PASSED]
call :end_cleanup
goto :eof

:test_failed
color c
echo [FAILED]
call :end_cleanup
exit /B 1

:end_cleanup
	cd "%ORIGINAL_DIR%"
	pause
	goto :eof
