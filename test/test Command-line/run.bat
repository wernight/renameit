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
if not exist test_files goto rmdir_end
	rmdir /s /q test_files
	if not errorlevel 1 goto rmdir_end
		echo Command failed: rmdir test_files
		goto failed
:rmdir_end
%RAR% x test_files.rar >NUL
cd test_files

rem ==================================================
rem = COPY THE FILES IN "BEFORE" TO A TEMP FOLDER    =
rem ==================================================
mkdir after.normal.generated
if not errorlevel 1 goto mkdir1_end
	echo Command failed: mkdir after.normal.generated
	goto failed
:mkdir1_end
xcopy before after.normal.generated /S /E /Q /H /K >NUL
if not errorlevel 1 goto xcopy1_end
	echo Command failed: xcopy before after.normal.generated
	goto failed
:xcopy1_end

mkdir after.recursive.generated
if not errorlevel 1 goto mkdir2_end
	echo Command failed: mkdir after.recursive.generated
	goto failed
:mkdir2_end
xcopy before after.recursive.generated /S /E /Q /H /K >NUL
if not errorlevel 1 goto xcopy2_end
	echo Command failed: xcopy before after.recursive.generated
	goto failed
:xcopy2_end

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
%COMPFOLD% after.normal.ref after.normal.generated
if errorlevel 1 goto failed

%COMPFOLD% after.recursive.ref after.recursive.generated
if errorlevel 1 goto failed

rem ===============
rem = TEST PASSED =
rem ===============
:passed
cd ..
rmdir /s /q test_files
color 2
echo *** No errors detected
exit /B

rem ===============
rem = TEST FAILED =
rem ===============
:failed
cd ..
set ALL_PASSED=0
color c
echo.
echo *** errors detected in test suite; see standard output for details
exit /B 1
