rem =======================
rem = CHECK THE ARGUMENTS =
rem =======================
if not '%1'=='' goto args_ok
	echo "Prodive the test folder name as first argument.
	exit /B 2
:args_ok
set TEST_NAME=%1

cd %TEST_NAME%

rem =================
rem = CONFIGURATION =
rem =================
set RENAMEIT=..\..\RenameIt.exe
set COMPFOLD=..\..\CompFold.exe

rem ======================
rem = DISPLAY TEST START =
rem ======================
echo Running %TEST_NAME%...

rem ===============================================
rem = COPY THE FILES IN "BEFORE" TO A TEMP FOLDER =
rem ===============================================
if exist after.generated rmdir /s /q after.generated
mkdir after.generated
xcopy before after.generated /S /E /Q /H /K >NUL

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
%COMPFOLD% after.ref after.generated
if errorlevel 1 goto failed

rmdir /s /q after.generated
goto passed

:passed
goto end

:failed
set ALL_PASSED=0
color c
goto end

:end
echo.
cd ..
