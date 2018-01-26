cd .\netData

for %%i in (*.proto) do (  
echo %%i
"..\protoc" --cpp_out=..\cpp %%i
)
echo end
pause
exit