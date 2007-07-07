@echo off

rem =================
rem = CONFIGURATION =
rem =================
set RENAMEIT=..\..\RenameIt.exe
set COMPFOLD=..\..\CompFold.exe
set RAR=..\Rar.exe

rem ==============
rem = UNCOMPRESS =
rem ==============
if exist test_files rmdir /s /q test_files
%RAR% x test_files.rar >NUL
cd test_files

rem ==================================================
rem = COPY THE FILES IN "BEFORE" TO A TEMP FOLDER    =
rem ==================================================
mkdir after.normal.generated
xcopy before after.normal.generated /S /E /Q /H /K >NUL

mkdir after.recursive.generated
xcopy before after.recursive.generated /S /E /Q /H /K >NUL

rem ==================================================
rem = RENAME USING OUR FILTER                        =
rem ==================================================
echo Running normal test...
%RENAMEIT% /x /f ..\filter.rit after.normal.generated /a
if not errorlevel 1 goto passed1a
	echo Rename-It! returned error code: %ERRORLEVEL%
	goto failed
:passed1a
echo.

echo Running recursive test...
%RENAMEIT% /x /f ..\filter.rit after.recursive.generated /r /a
if not errorlevel 1 goto passed1b
	echo Rename-It! returned error code: %ERRORLEVEL%
	goto failed
:passed1b
echo.

rem ==================================================
rem = COMPARE THE RESULT WITH THE EXPECTED RESULT    =
rem ==================================================
%COMPFOLD% after.normal.ref after.normal.generated >NUL
if errorlevel 1 goto failed

%COMPFOLD% after.recursive.ref after.recursive.generated >NUL
if errorlevel 1 goto failed

rem ===============
rem = TEST PASSED =
rem ===============
:passed
cd ..
rmdir /s /q test_files
color 2
echo *** No errors detected
exit /B 1

rem ===============
rem = TEST FAILED =
rem ===============
:failed
cd ..
set ALL_PASSED=0
color c
echo.
echo *** errors detected in test suite; see standard output for details
exit /B
