@echo off

start "LogServer"			"G_LogServer.exe" 
start "CenterServer"		"G_CenterServer.exe" 
start "DBServer"			"G_DBServer.exe" 
start "LoginServer"			"G_LoginServer.exe" 
start "[1线]GameServer"		"G_GameServer.exe" 1
start "[1线]GameGateway"	"G_GameGateway.exe" 1
start "[2线]GameServer"		"G_GameServer.exe" 2
start "[2线]GameGateway"	"G_GameGateway.exe" 2

::start "Robot" "G_Robot.exe" 