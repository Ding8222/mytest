/*
* 服务器负载管理
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>
#include "GlobalDefine.h"

struct ServerStatusInfo
{
	ServerStatusInfo()
	{
		nServerID = 0;
		nServerType = 0;
		nMaxClient = 0;
		nNowClient = 0;
		memset(chIP, 0, MAX_IP_LEN);
		nPort = 0;
		nSubServerID = 0;
	}
	int nServerID;
	int nServerType;
	int nMaxClient;
	int nNowClient;
	char chIP[MAX_IP_LEN];
	int nPort;
	int nSubServerID;
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

	void AddNewServer(serverinfo *info, Msg *pMsg);
	void UpdateServerLoad(int id, int clientcountnow, int clientcountmax);
	void DelServerByGameID(int id);

	// 根据负载返回玩家应该进去的服务器
	// 传入的为申请的ServerID
	ServerStatusInfo *GetGateInfoByServerID(int id);
private:

	// <gameid,info>
	std::unordered_map<int, ServerStatusInfo *> m_ServerInfo;
	// <serverid,gateid>
	std::unordered_map<int, int> m_GateServerInfo;
};