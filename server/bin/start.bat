@echo off

start "[Game]LogServer"		"LogServer.exe" 
start "[Game]CenterServer"	"CenterServer.exe" 
start "[Game]DBServer"		"DBServer.exe" 
start "[Game]LoginServer"	"LoginServer.exe" 
start "[Game]GameServer"	"GameServer.exe" 1
start "[Game]GameGateway"	"GameGateway.exe" 1
start "[Game]GameServer"	"GameServer.exe" 2
start "[Game]GameGateway"	"GameGateway.exe" 2

start "[Game]Robot" "Robot.exe" 
