@echo off

start "[Game]CenterServer"	"CenterServer.exe" 
start "[Game]DBServer"	"DBServer.exe" 
start "[Game]LoginServer"	"LoginServer.exe" 
start "[Game]GameServer"	"GameServer.exe" 
start "[Game]GameGateway"	"GameGateway.exe" 

start "[Game]Robot" "Robot.exe" 