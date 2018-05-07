#pragma once
#include <string>
#include "BaseConfig.h"
#include "platform_config.h"

#define Config CConfig::Instance()
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

private:
	int32 m_DBID;

};