@echo off
cd .\netData

for %%i in (*.proto) do (
echo %%i
"..\protoc.exe" -o ..\lua\%%i.pb %%i
)

pause
exit