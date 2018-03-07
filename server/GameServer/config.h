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

	bool Init(const char *servername = "", int lineid = 0);
	void SetLineID(int id) { m_LineID = id; }
	int GetLineID() { return m_LineID; }
	std::string GetCenterServerIP() { return s_CenterServerIP; }
	int GetCenterServerPort() { return m_CenterServerPort; }
	int GetCenterServerID() { return m_CenterServerID; }

private:
	int m_LineID;

	std::string s_CenterServerIP;
	int m_CenterServerPort;
	int m_CenterServerID;
};