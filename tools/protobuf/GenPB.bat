SETLOCAL ENABLEDELAYEDEXPANSION
@echo off
cd .\netData
for /f "delims=/" %%i in ('git status -s') do (  
SET filename=%%i
echo !filename:~3!|findstr "proto"&&"..\protoc" --cpp_out=..\cpp !filename:~3!
echo !filename:~3!|findstr "proto">nul &&"..\protoc.exe" -o ..\lua\!filename:~3,-6!.pb !filename:~3!
)

cd ..
xcopy "cpp\*.h" "..\..\server\Base\PB" /S /Y
xcopy "cpp\*.cc" "..\..\server\Base\PB" /S /Y

del /s /Q "cpp\*.h"
del /s /Q "cpp\*.cc"

pause
exit
