#pragma once
#include <string>
#include"BaseConfig.h"

#define Config CConfig::Instance()
class CConfig :public CBaseConfig
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

	bool GetIsOpenSQLLog() { return m_SQLLog; }
private:

	bool m_SQLLog;
};