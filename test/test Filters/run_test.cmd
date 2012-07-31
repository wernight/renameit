@echo off
::
:: NOTE:
:: You can Drag & Drop a zip file on this batch to run only that test.
::
setlocal enabledelayedexpansion

:::::::::::::::::::
:: CONFIGURATION ::
:::::::::::::::::::
set script_dir=%~dp0
set renameit=%script_dir%..\RenameIt.exe
set compfold=%script_dir%..\..\tools\CompFold.exe
set unzip=%script_dir%..\..\tools\7za.exe
set echoc=%script_dir%..\..\tools\echoc.exe

:::::::::::::::::::::::::
:: CHECK THE ARGUMENTS ::
:::::::::::::::::::::::::
if not exist "%script_dir%tmp" mkdir "%script_dir%tmp"
pushd "%script_dir%tmp"

set total_tests=0
set failed_tests=0

if not "%~1" == "" (
    call :run_test "%~1"
) else (
    for %%i in ("%script_dir%*.7z") do (
        call :run_test "%%~fi"
    )
)

if "%failed_tests%" == "0" (
    "%echoc%" 0 10 *** No errors detected^^
) else (
    "%echoc%" 0 12 *** %failed_tests%/%total_tests% tests failed^^
    "%echoc%" 0 12 *** errors detected in test suite; see standard output for details^^
)

popd
if not "%IN_RUN_ALL_TESTS%" == "1" pause
exit /B %failed_tests%

:run_test test_file
    set test_name=%~n1
    set test_file=%~f1
    set /A total_tests=%total_tests%+1

    if not exist "%test_file%" (
        echo Error: Cannot find the test file `%test_file%`.
        echo.
    )

    ::::::::::::::::::::::::
    :: DISPLAY TEST START ::
    ::::::::::::::::::::::::
    "%echoc%" 0 7 "Running %test_name%... "

    ::::::::::::::::
    :: UNCOMPRESS ::
    ::::::::::::::::
    if exist "%test_name%" (
        rmdir /s /q "%test_name%" || (
            call :failed
            echo Command failed: rmdir "%test_name%"
            goto :eof
        )
    )
    "%unzip%" x "%test_file%" >NUL || (
        call :failed
        echo Command failed: 7za.exe x %test_name%.7z
        goto :eof
    )
    pushd "%test_name%"

    :::::::::::::::::::::::::::::::::::::::::::::::::
    :: COPY THE FILES IN "BEFORE" TO A TEMP FOLDER ::
    :::::::::::::::::::::::::::::::::::::::::::::::::
    robocopy /MIR /R:0 before after.generated >NUL
    if errorlevel 4 (
        call :failed
        echo Command failed: robocopy /MIR before after.generated
        popd
        goto :eof
    )

    ::::::::::::::::::::::::::::::::::::::::::::::::::::
    :: RENAME USING OUR FILTER                        ::
    ::::::::::::::::::::::::::::::::::::::::::::::::::::
    "%renameit%" /x /f filter.rit after.generated /r /a || (
        call :failed
        echo Rename-It! returned error code: %ERRORLEVEL%
        popd
        goto :eof
    )

    ::::::::::::::::::::::::::::::::::::::::::::::::::::
    :: COMPARE THE RESULT WITH THE EXPECTED RESULT    ::
    ::::::::::::::::::::::::::::::::::::::::::::::::::::
    "%compfold%" after.ref after.generated >NUL
    if errorlevel 1 (
        call :failed
        if not "%IN_RUN_ALL_TESTS%" == "1" (
            "%compfold%" after.ref after.generated
            echo.
        )
        popd
        goto :eof
    )

    call :passed
    popd
    rmdir /s /q "%test_name%"
    goto :eof
 
:passed
    "%echoc%" 0 10 [PASSED]
    echo.
    exit /B 0

:failed
    set /A failed_tests=%failed_tests%+1
    "%echoc%" 0 12 [FAILED]
    echo.
    exit /B 0
