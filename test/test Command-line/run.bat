@echo off

rem =================
rem = CONFIGURATION =
rem =================
set RENAMEIT=..\RenameIt.exe
set COMPFOLD=..\CompFold.exe

rem ==================================================
rem = COPY THE FILES IN "BEFORE" TO A TEMP FOLDER    =
rem ==================================================
if exist after.normal.generated rmdir /s /q after.normal.generated
mkdir after.normal.generated
xcopy before after.normal.generated /S /E /Q /H /K >NUL

if exist after.recursive.generated rmdir /s /q after.recursive.generated
mkdir after.recursive.generated
xcopy before after.recursive.generated /S /E /Q /H /K >NUL

rem ==================================================
rem = RENAME USING OUR FILTER                        =
rem ==================================================
echo Running /R recursive test...
%RENAMEIT% /x /f filter.rit after.normal.generated /a
if not errorlevel 1 goto passed1a
	call %SHOW_TEST_RESULT% %TEST_NAME% 0
	echo Rename-It! returned error code: %ERRORLEVEL%
	echo.
	exit /b 1
:passed1a

%RENAMEIT% /x /f filter.rit after.recursive.generated /r /a
if not errorlevel 1 goto passed1b
	call %SHOW_TEST_RESULT% %TEST_NAME% 0
	echo Rename-It! returned error code: %ERRORLEVEL%
	echo.
	exit /b 1
:passed1b

rem ==================================================
rem = COMPARE THE RESULT WITH THE EXPECTED RESULT    =
rem ==================================================
%COMPFOLD% after.normal.ref after.normal.generated >NUL
if not errorlevel 1 goto passed2a
	call %SHOW_TEST_RESULT% %TEST_NAME% 0
	%COMPFOLD% after.normal.ref after.normal.generated
	exit /b 1
:passed2a

%COMPFOLD% after.recursive.ref after.recursive.generated >NUL
if not errorlevel 1 goto passed2b
	call %SHOW_TEST_RESULT% %TEST_NAME% 0
	%COMPFOLD% after.recursive.ref after.recursive.generated
	exit /b 1
:passed2b

rem ===============
rem = TEST PASSED =
rem ===============
:passed
rmdir /s /q after.normal.generated
rmdir /s /q after.recursive.generated
color 2
echo.
echo *** No errors detected
goto end

rem ===============
rem = TEST FAILED =
rem ===============
:failed
set ALL_PASSED=0
color c
echo.
echo *** errors detected in test suite; see standard output for details
goto end

rem =====================
rem = End               =
rem =====================
:end
pause
