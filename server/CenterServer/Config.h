#pragma once
#include <string>
#include "BaseConfig.h"
#include "platform_config.h"

class CConfig:public CBaseConfig
{
public:
	CConfig();
	~CConfig();

	static CConfig &Instance()
	{
		static CConfig m;
		return m;
	}

	bool Init(const char *servername = "");
	int32 GetDBID() { return m_DBID; }

	int GetNameCheckServerID() { return m_NameCheckServerID; }
	int GetNameCheckServerPort() { return m_NameCheckServerPort; }
	char *GetNameCheckServerIP() { return s_NameCheckServerIP; }
private:
	int32 m_DBID;

	int m_NameCheckServerID;
	int m_NameCheckServerPort;
	char s_NameCheckServerIP[MAX_IP_LEN];
};