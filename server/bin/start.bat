@echo off

start "NameCheckServer"		"G_NameCheckServer.exe" 
start "LogServer"			"G_LogServer.exe" 
start "CenterServer"		"G_CenterServer.exe" 
start "DBServer"			"G_DBServer.exe" 
start "LoginServer"			"G_LoginServer.exe" 
start "[1��]GameServer"		"G_GameServer.exe" 1
start "[1��]GameGateway"	"G_GameGateway.exe" 1
start "[2��]GameServer"		"G_GameServer.exe" 2
start "[2��]GameGateway"	"G_GameGateway.exe" 2
start "[3��]GameServer"		"G_GameServer.exe" 3
start "[3��]GameGateway"	"G_GameGateway.exe" 3
start "[4��]GameServer"		"G_GameServer.exe" 4
start "[4��]GameGateway"	"G_GameGateway.exe" 4
start "[5��]GameServer"		"G_GameServer.exe" 5
start "[5��]GameGateway"	"G_GameGateway.exe" 5

::start "Robot" "G_Robot.exe" 