#pragma once
#include <string>
#include <list>
#include "BaseConfig.h"
#include "lxnet\base\platform_config.h"

struct GameSvr
{
	GameSvr()
	{
		id = 0;
		port = 0;
		ip.clear();
	}

	int32 id;
	int32 port;
	std::string ip;
	std::string name;
};

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

	bool Init(const char *servername = "", int lineid = 0);

	void SetLineID(int id) { m_LineID = id; }
	int GetLineID() { return m_LineID; }

	int GetMaxClientNum() { return m_MaxClientNum; }
	int GetRecvDataLimt() { return m_RecvDataLimt; }
	int GetSendDataLimt() { return m_SendDataLimt; }
	bool IsOpenClientConnectLog() { return m_IsOpenClientConnectLog; }

	std::list<GameSvr *> &GetGameSvrList() { return m_GameSvrList; }
private:
	int m_LineID;

	int m_MaxClientNum;
	int m_RecvDataLimt;
	int m_SendDataLimt;
	bool m_IsOpenClientConnectLog;

	std::list<GameSvr *> m_GameSvrList;
};