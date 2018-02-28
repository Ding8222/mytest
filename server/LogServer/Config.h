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

	std::string GetDBName() { return s_DBName; }
	std::string GetDBUser() { return s_DBUser; }
	std::string GetDBPass() { return s_DBPass; }
	std::string GetDBIP() { return s_DBIP; }
	bool GetIsOpenSQLLog() { return m_SQLLog; }
private:

	std::string s_DBName;
	std::string s_DBUser;
	std::string s_DBPass;
	std::string s_DBIP;

	bool m_SQLLog;
};