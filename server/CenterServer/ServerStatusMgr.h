/*
* 服务器负载管理
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>

struct ServerInfo
{
	ServerInfo()
	{
		nServerID = 0;
		nServerType = 0;
		nMaxClient = 0;
		nNowClient = 0;
		memset(chIP, 0, MAX_IP_LEN);
		nPort = 0;
		nGateID = 0;
		memset(chGateIP, 0, MAX_IP_LEN);
		nGatePort = 0;
	}
	int nServerID;
	int nServerType;
	int nMaxClient;
	int nNowClient;
	char chIP[MAX_IP_LEN];
	int nPort;
	int nGateID;
	char chGateIP[MAX_IP_LEN];
	int nGatePort;
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

	bool AddNewServer(ServerInfo *info);
	void UpdateServerLoad(int id, int clientcountnow, int clientcountmax);
	void DelServerByGameID(int id);
	void DelServerByGateID(int id);

	// 根据负载返回玩家应该进去的服务器
	// 传入的为申请的ServerID
	ServerInfo *GetGateInfoByServerID(int id);
private:

	// <gameid,info>
	std::unordered_map<int, ServerInfo *> m_ServerInfo;
	// <gateid,info>
	std::unordered_map<int, ServerInfo *> m_GateServerInfo;
};