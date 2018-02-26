/*
* 基本配置
* Copyright (C) ddl
* 2018
*/
#pragma once

class CBaseConfig
{
public:
	CBaseConfig();
	~CBaseConfig();
	
	virtual bool Init(const char *servername = "");

	bool IsOpenElapsedLog() { return m_IsOpenElapsedLog; }
	int GetListenPort() { return m_ListenPort; }
	int GetOverTime() { return m_OverTime; }
	int GetPingTime() { return m_PingTime; }
	int GetServerID() { return m_ServerID; }
	int GetServerType() { return m_ServerType; }
	int SetServerType(int nType) { return m_ServerType = nType; }
	std::string GetServerIP() { return s_ServerIP; }

	int GetLogServerID() { return m_LogServerID; }
	int GetLogServerPort() { return m_LogServerPort; }
	std::string GetLogServerIP() { return s_LogServerIP; }
private:
	bool m_IsOpenElapsedLog;
	int m_ListenPort;
	int m_OverTime;
	int m_PingTime;
	int m_ServerID;
	int m_ServerType;
	std::string s_ServerIP;

	int m_LogServerID;
	int m_LogServerPort;
	std::string s_LogServerIP;
};