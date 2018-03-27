#pragma once
#include <string>
#include"BaseConfig.h"

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

	int GetBeginMap() { return m_BeginMapID; }
	bool GetIsOpenSQLLog() { return m_SQLLog; }

private:
	int m_BeginMapID;
	bool m_SQLLog;
};