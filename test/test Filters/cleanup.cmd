@echo off

echo Cleaning up the test folders...
for %%d in (*.rar) do call :cleanup_test "%%~nd"
goto :eof

:cleanup_test
set TEST_NAME=%~n1

if not exist "%TEST_NAME%" goto rmdir_end
	rmdir /s /q "%TEST_NAME%"
	if not errorlevel 1 goto rmdir_end
		echo Command failed: rmdir "%TEST_NAME%"
		exit /B 1
:rmdir_end
exit /B 0
