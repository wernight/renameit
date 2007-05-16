@echo off

rem ==================================
rem = Simply execute all the run.bat =
rem ==================================

set ALL_PASSED=1
for /D %%d in (*.*) do call .run_test.bat "%%d"
if not %ALL_PASSED%==1 goto one_failed

rem =====================
rem = All test passed   =
rem =====================
echo *** No errors detected
color 2
goto end

rem =====================
rem = One test failed   =
rem =====================
:one_failed
echo *** errors detected in test suite; see standard output for details
color c
goto end

rem =====================
rem = End               =
rem =====================
:end
pause
