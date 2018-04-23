@echo off
start ComLua.bat
start /wait ComPB.bat
xcopy "cpp" "..\..\server\Base\PB" /S /Y
del /s /Q "cpp"
pause
