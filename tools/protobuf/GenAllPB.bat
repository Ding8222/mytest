SETLOCAL ENABLEDELAYEDEXPANSION
@echo off
cd .\netData

for %%i in (*.proto) do (  
echo %%i
SET filename=%%i
"..\protoc" --cpp_out=..\cpp %%i
"..\protoc" -o ..\lua\!filename:~0,-6!.pb %%i
)
echo end

cd ..
xcopy "cpp\*.h" "..\..\server\Base\PB" /S /Y
xcopy "cpp\*.cc" "..\..\server\Base\PB" /S /Y

del /s /Q "cpp\*.h"
del /s /Q "cpp\*.cc"
pause
