@echo off

echo ==========================================
echo TEST COMMAND-LINE
echo ==========================================
cd "test Command-line"
call run.bat
cd ..
echo.

echo ==========================================
echo TEST FILTERS
echo ==========================================
cd "test Filters"
call run.bat
cd ..
echo.

pause
