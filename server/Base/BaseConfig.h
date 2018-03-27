/*
* 基本配置
* Copyright (C) ddl
* 2018
*/
#pragma once

#include <string>
#include "platform_config.h"

class CBaseConfig
{
public:
	CBaseConfig();
	~CBaseConfig();
	
	bool Init(const std::string &servername, int32 lineid = 0);

	int32 GetGroupID() { return m_GroupID; }
	int32 GetOverTime() { return m_OverTime; }
	int32 GetPingTime() { return m_PingTime; }
	bool IsOpenElapsedLog() { return m_IsOpenElapsedLog; }

	int32 GetServerID() { return m_ServerID; }
	int32 GetListenPort() { return m_ListenPort; }
	void SetServerType(int32 nType) { m_ServerType = nType; }
	int32 GetServerType() { return m_ServerType; }
	void SetServerName(const char *name) { m_ServerName = name; }
	const char *GetServerName() { return m_ServerName.c_str(); }
	const char *GetServerIP() { return m_ServerIP.c_str(); }

	int32 GetLogServerID() { return m_LogServerID; }
	int32 GetLogServerPort() { return m_LogServerPort; }
	const char *GetLogServerIP() { return m_LogServerIP.c_str(); }

	int32 GetCenterServerID() { return m_CenterServerID; }
	int32 GetCenterServerPort() { return m_CenterServerPort; }
	const char *GetCenterServerIP() { return m_CenterServerIP.c_str(); }


	int32 GetNameCheckServerID() { return m_NameCheckServerID; }
	int32 GetNameCheckServerPort() { return m_NameCheckServerPort; }
	const char *GetNameCheckServerIP() { return m_NameCheckServerIP.c_str(); }

	const char *GetDBName() { return m_DBName.c_str(); }
	const char *GetDBUser() { return m_DBUser.c_str(); }
	const char *GetDBPass() { return m_DBPass.c_str(); }
	const char *GetDBIP() { return m_DBIP.c_str(); }

	int32 GetMonitorPort() { return m_MonitorPort; }
private:
	int32 m_GroupID;
	int32 m_OverTime;
	int32 m_PingTime;
	// 是否开启超时log
	bool m_IsOpenElapsedLog;

	// 本地服务器信息
	int32 m_ServerID;
	int32 m_ListenPort;
	int32 m_ServerType;
	std::string m_ServerName;
	// 外网IP
	std::string m_ServerIP;

	// log服务器信息
	int32 m_LogServerID;
	int32 m_LogServerPort;
	std::string m_LogServerIP;

	// 中心服务器信息
	int32 m_CenterServerID;
	int32 m_CenterServerPort;
	std::string m_CenterServerIP;

	// 名称检查服务器
	int32 m_NameCheckServerID;
	int32 m_NameCheckServerPort;
	std::string m_NameCheckServerIP;

	// 数据库信息
	std::string m_DBName;
	std::string m_DBUser;
	std::string m_DBPass;
	std::string m_DBIP;

	// 监控端口
	int32 m_MonitorPort;
};