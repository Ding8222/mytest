/*
* 基本配置
* Copyright (C) ddl
* 2018
*/
#pragma once

#include "GlobalDefine.h"
class CBaseConfig
{
public:
	CBaseConfig();
	~CBaseConfig();
	
	bool Init(const std::string &servername, int lineid = 0);

	bool IsOpenElapsedLog() { return m_IsOpenElapsedLog; }
	int GetListenPort() { return m_ListenPort; }
	int GetOverTime() { return m_OverTime; }
	int GetPingTime() { return m_PingTime; }
	int GetServerID() { return m_ServerID; }
	int GetServerType() { return m_ServerType; }
	int SetServerType(int nType) { return m_ServerType = nType; }
	char *GetServerIP() { return s_ServerIP; }

	int GetLogServerID() { return m_LogServerID; }
	int GetLogServerPort() { return m_LogServerPort; }
	char *GetLogServerIP() { return s_LogServerIP; }

private:
	bool m_IsOpenElapsedLog;
	int m_ListenPort;
	int m_OverTime;
	int m_PingTime;
	int m_ServerID;
	int m_ServerType;
	char s_ServerIP[MAX_IP_LEN];

	int m_LogServerID;
	int m_LogServerPort;
	char s_LogServerIP[MAX_IP_LEN];
};