@echo off
rem =======================
rem = CHECK THE ARGUMENTS =
rem =======================
if '%1'=='' goto args_missing
if exist "%~n1.rar" goto args_ok
	echo Error: Cannot find the test file `%~n1.rar`.
	echo.
:args_missing
	echo Provide the name of the test as first argument.
	echo Example: To run the test "Series.rar" provide "Series.rar" as first argument.
	pause
	exit /B 2
:args_ok

rem =================
rem = CONFIGURATION =
rem =================
set TEST_NAME=%~n1
set RENAMEIT=..\..\RenameIt.exe
set COMPFOLD=..\..\CompFold.exe
set RAR=..\Rar.exe

rem ======================
rem = DISPLAY TEST START =
rem ======================
echo Running %TEST_NAME%...

rem ==============
rem = UNCOMPRESS =
rem ==============

if not exist "%TEST_NAME%" goto rmdir_end
	rmdir /s /q "%TEST_NAME%"
	if not errorlevel 1 goto rmdir_end
		echo Command failed: rmdir "%TEST_NAME%"
		goto failed
:rmdir_end
%RAR% x "%TEST_NAME%.rar" >NUL
cd %TEST_NAME%

rem ===============================================
rem = COPY THE FILES IN "BEFORE" TO A TEMP FOLDER =
rem ===============================================
mkdir after.generated
if not errorlevel 1 goto mkdir_end
	echo Command failed: mkdir after.generated
	goto failed
:mkdir_end
xcopy before after.generated /S /E /Q /H /K >NUL
if not errorlevel 1 goto xcopy_end
	echo Command failed: xcopy before after.generated
	goto failed
:xcopy_end

rem ==================================================
rem = RENAME USING OUR FILTER                        =
rem ==================================================
%RENAMEIT% /x /f filter.rit after.generated /r /a
if not errorlevel 1 goto passed1
	echo Rename-It! returned error code: %ERRORLEVEL%
	goto failed
:passed1

rem ==================================================
rem = COMPARE THE RESULT WITH THE EXPECTED RESULT    =
rem ==================================================
if '%IN_RUN_ALL_TESTS%'=='1' %COMPFOLD% after.ref after.generated >NUL
if not '%IN_RUN_ALL_TESTS%'=='1' %COMPFOLD% after.ref after.generated
if errorlevel 1 goto failed
goto passed

:passed
cd ..
rmdir /s /q "%TEST_NAME%"
color 2
if '%IN_RUN_ALL_TESTS%'=='1' echo                                         [PASSED]
if not '%IN_RUN_ALL_TESTS%'=='1' echo.
if not '%IN_RUN_ALL_TESTS%'=='1' echo *** No errors detected
goto end

:failed
cd ..
set /A FAILED_TESTS=%FAILED_TESTS%+1
color c
if '%IN_RUN_ALL_TESTS%'=='1' echo                                         [FAILED]
if not '%IN_RUN_ALL_TESTS%'=='1' echo.
if not '%IN_RUN_ALL_TESTS%'=='1' echo *** errors detected in test suite; see standard output for details
goto end

:end
echo.
if not '%IN_RUN_ALL_TESTS%'=='1' pause
