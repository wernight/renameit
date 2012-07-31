@echo off
setlocal enabledelayedexpansion

:::::::::::::::::::
:: CONFIGURATION ::
:::::::::::::::::::
set script_dir=%~dp0
set renameit=%script_dir%..\RenameIt.exe
set compfold=%script_dir%..\..\tools\CompFold.exe
set unzip=%script_dir%..\..\tools\7za.exe
set echoc=%script_dir%..\..\tools\echoc.exe

::::::::::::::::
:: UNCOMPRESS ::
::::::::::::::::
if not exist "%script_dir%tmp" mkdir "%script_dir%tmp"
pushd "%script_dir%tmp"

if exist test_files (
	rmdir /s /q test_files || (
		echo Command failed: rmdir test_files
		goto failed
	)
)
"%unzip%" x "%script_dir%test_files.7z" >NUL || (
    call :failed
    echo Command failed: 7za.exe x test_files.7z
    popd
    goto :eof
)

:::::::::::::::::::
:: RUN THE TESTS ::
:::::::::::::::::::
pushd test_files

set failed_tests=0

echo Running normal test...
call :run_test after.normal.ref /a

echo Running recursive test...
call :run_test after.recursive.ref /r /a

popd

if "%failed_tests%" == "0" (
    rmdir /s /q test_files
    "%echoc%" 0 10 *** No errors detected^^
) else (
    "%echoc%" 0 12 *** %failed_tests%/2 tests failed^^
    "%echoc%" 0 12 *** errors detected in test suite; see standard output for details^^
)

popd
if not "%IN_RUN_ALL_TESTS%" == "1" pause
exit /B %failed_tests%

:run_test reference_folder rit_arg1 rit_arg2...
    set reference_folder=%~1
    shift

    robocopy /MIR /R:0 before after.generated >NUL
    if errorlevel 4 (
        echo Command failed: robocopy /MIR before after.generated
        goto failed
    )

    ::::::::::::::::::::::::::::::::::::::::::::::::::::
    :: RENAME USING OUR FILTER AND GIVEN ARGUMENTS    ::
    ::::::::::::::::::::::::::::::::::::::::::::::::::::
    "%echoc%" 0 7 ^> "RenameIt.exe /x /f filter.rit after.generated %* "
    "%renameit%" /x /f "%script_dir%filter.rit" after.generated %1 %2 %3 %4 %5 %6 %7 %8 %9 || (
        call :failed
        echo Rename-It! returned error code: %ERRORLEVEL%
        goto :eof
    )

    ::::::::::::::::::::::::::::::::::::::::::::::::::::
    :: COMPARE THE RESULT WITH THE EXPECTED RESULT    ::
    ::::::::::::::::::::::::::::::::::::::::::::::::::::
    "%compfold%" "%reference_folder%" after.generated >NUL
    if errorlevel 1 (
        call :failed
        if not "%IN_RUN_ALL_TESTS%" == "1" (
            "%compfold%" "%reference_folder%" after.generated
            echo.
        )
    ) else (
        call :passed
    )
    goto :eof

:::::::::::::::::
:: TEST PASSED ::
:::::::::::::::::
:passed
    "%echoc%" 0 10 [PASSED]
    echo.
	exit /B 0

:::::::::::::::::
:: TEST FAILED ::
:::::::::::::::::
:failed
    set /A failed_tests=%failed_tests%+1
    "%echoc%" 0 12 [FAILED]
    echo.
	exit /B 0
