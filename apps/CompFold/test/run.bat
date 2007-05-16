@echo off

rem Assert f1==f1
CompFold.exe f1 f1
if errorlevel 1 goto test_failed

rem Assert f2==f2
CompFold.exe f2 f2
if errorlevel 1 goto test_failed

rem Assert f1!=f2 by 8 files
CompFold.exe f1 f2
if errorlevel 9 goto test_failed
if not errorlevel 8 goto test_failed

color 2
echo [PASSED]
goto end

:test_failed
color c
echo [FAILED]
goto end

:end
pause
exit
