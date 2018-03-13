/*
* 服务器负载管理
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>
#include <list>
#include "GlobalDefine.h"

struct ServerStatusInfo
{
	ServerStatusInfo()
	{
		nLineID = 0;
		nServerID = 0;
		memset(chIP, 0, MAX_IP_LEN);
		nPort = 0;
		nMaxClient = 0;
		nNowClient = 0;
	}
	int32 nLineID;
	int32 nServerID;
	char chIP[MAX_IP_LEN];
	int32 nPort;
	int32 nMaxClient;
	int32 nNowClient;
};

struct stServerInfo
{
	stServerInfo()
	{
		nLineiD = 0;
		nServerID = 0;
	}

	int32 nLineiD;
	int32 nServerID;
};

class CServerStatusMgr
{
public:
	CServerStatusMgr();
	~CServerStatusMgr();

	static CServerStatusMgr &Instance()
	{
		static CServerStatusMgr m;
		return m;
	}

	void Destroy();

	void AddGameServer(serverinfo *info, Msg *pMsg);
	void AddGateServer(serverinfo *info, Msg *pMsg);
	void UpdateGameServerLoad(int32 id, int32 clientcountnow, int32 clientcountmax);
	void UpdateGateServerLoad(int32 id, int32 clientcountnow, int32 clientcountmax);
	void DelGameServer(int32 serverid);
	void DelGateServer(int32 serverid);
	
	ServerStatusInfo *GetGameServerInfo(int32 id, int32 lineid = 0);
	ServerStatusInfo *GetGateServerInfo();
private:

	// 注册了的服务器的信息
	// <serverid,gateid>
	std::unordered_map<int32, ServerStatusInfo *> m_GameServerInfo;
	std::unordered_map<int32, ServerStatusInfo *> m_GateServerInfo;
	// <mapid,serverid>
	std::unordered_map<int32, std::list<stServerInfo>> m_ServerMapInfo;
};
