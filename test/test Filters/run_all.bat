@echo off

rem ==================================
rem = Simply execute all the run.bat =
rem ==================================

set IN_RUN_ALL_TESTS=1
set FAILED_TESTS=0
for %%d in (*.rar) do call run_test.bat "%%~nd"
if not %FAILED_TESTS%==0 goto one_failed

rem =====================
rem = All test passed   =
rem =====================
echo *** No errors detected
color 2
if not '%IN_RUN_ALL%'=='1' pause
exit /B

rem =====================
rem = One test failed   =
rem =====================
:one_failed
set TOTAL_TESTS=0
for %%d in (*.rar) do set /A TOTAL_TESTS=%TOTAL_TESTS%+1
echo *** %FAILED_TESTS%/%TOTAL_TESTS% tests failed
echo *** errors detected in test suite; see standard output for details
color c
if not '%IN_RUN_ALL%'=='1' pause
exit /B 1
