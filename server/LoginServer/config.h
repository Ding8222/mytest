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

	int GetMaxClientNum() { return m_MaxClientNum; }
	int GetRecvDataLimt() { return m_RecvDataLimt; }
	int GetSendDataLimt() { return m_SendDataLimt; }
	bool IsOpenClientConnectLog() { return m_IsOpenClientConnectLog; }

private:

	int m_MaxClientNum;
	int m_RecvDataLimt;
	int m_SendDataLimt;
	bool m_IsOpenClientConnectLog;
};