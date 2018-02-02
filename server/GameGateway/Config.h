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
	std::string GetCenterServerIP() { return s_CenterServerIP; }
	int GetCenterServerPort() { return m_CenterServerPort; }
	int GetCenterServerID() { return m_CenterServerID; }
	std::string GetGameServerIP() { return s_GameServerIP; }
	int GetGameServerPort() { return m_GameServerPort; }
	int GetGameServerID() { return m_GameServerID; }
	int GetMaxClientNum() { return m_MaxClientNum; }
	int GetRecvDataLimt() { return m_RecvDataLimt; }
	int GetSendDataLimt() { return m_SendDataLimt; }

private:

	std::string s_CenterServerIP;
	int m_CenterServerPort;
	int m_CenterServerID;

	std::string s_GameServerIP;
	int m_GameServerPort;
	int m_GameServerID;

	int m_MaxClientNum;
	int m_RecvDataLimt;
	int m_SendDataLimt;
};