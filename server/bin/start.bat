@echo off

start "CenterServer"	"CenterServer.exe" 
start "DBServer"	"DBServer.exe" 
start "LoginServer"	"LoginServer.exe" 
start "GameServer"	"GameServer.exe" 
start "GameGateway"	"GameGateway.exe" 

::start "Robot" "Robot.exe" 