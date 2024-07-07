

set VAR1=%~1

echo %VAR1%

::for /f %%i in (%VAR1%\*) do 
::echo %%~i
for /f "delims=" %%a in ('dir /b "%VAR1%"') do (
	set VAR2=%~dp0\hashutils\bin.x86-64\md5sum %%~fa
)	

echo 