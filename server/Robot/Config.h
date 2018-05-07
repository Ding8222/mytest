#pragma once
#include <string>
#include"BaseConfig.h"

#define Config CConfig::Instance()
class CConfig
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
	std::string GetLoginServerIP() { return s_LoginServerIP; }
	int GetLoginServerPort() { return m_LoginServerPort; }
	int GetLoginServerID() { return m_LoginServerID; }
	int GetMaxRobot() { return m_MaxRobot; }
	int GetOverTime() { return m_OverTime; }
	int GetPingTime() { return m_PingTime; }

private:

	std::string s_LoginServerIP;
	int m_LoginServerPort;
	int m_LoginServerID;
	int m_MaxRobot;

	int m_OverTime;
	int m_PingTime;
};