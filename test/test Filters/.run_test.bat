rem =======================
rem = CHECK THE ARGUMENTS =
rem =======================
if not '%1'=='' goto args_ok
	echo "Prodive the test folder name as first argument.
	exit /B 2
:args_ok

rem =================
rem = CONFIGURATION =
rem =================
set TEST_NAME=%~1
set RENAMEIT=..\..\RenameIt.exe
set COMPFOLD=..\..\CompFold.exe
set RAR=..\Rar.exe

rem ==============
rem = UNCOMPRESS =
rem ==============

if exist "%TEST_NAME%" rmdir /s /q "%TEST_NAME%"
%RAR% x "%TEST_NAME%.rar" >NUL
cd %TEST_NAME%

rem ======================
rem = DISPLAY TEST START =
rem ======================
echo Running %TEST_NAME%...

rem ===============================================
rem = COPY THE FILES IN "BEFORE" TO A TEMP FOLDER =
rem ===============================================
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
goto passed

:passed
cd ..
rmdir /s /q "%TEST_NAME%"
goto end

:failed
cd ..
set ALL_PASSED=0
color c
goto end

:end
echo.
