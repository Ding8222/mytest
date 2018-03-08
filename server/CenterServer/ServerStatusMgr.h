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
		nServerType = 0;
		nMaxClient = 0;
		nNowClient = 0;
		memset(chIP, 0, MAX_IP_LEN);
		nPort = 0;
		nSubServerID = 0;
	}
	int32 nLineID;
	int32 nServerID;
	int32 nServerType;
	int32 nMaxClient;
	int32 nNowClient;
	char chIP[MAX_IP_LEN];
	int32 nPort;
	int32 nSubServerID;
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

	void AddNewServer(serverinfo *info, Msg *pMsg);
	void UpdateServerLoad(int32 id, int32 clientcountnow, int32 clientcountmax);
	void DelServerID(int32 serverid);

	// 根据Server返回Gate的信息
	ServerStatusInfo *GetGateInfoByServerID(int32 id);
	// 根据负载返回玩家应该进去的服务器
	// 传入的为申请的MapID,线路ID（为0的时候，自动分配）
	ServerStatusInfo *GetGateInfoByMapID(int32 id, int32 lineid = 0);
private:

	// <gameid,info>
	std::unordered_map<int32, ServerStatusInfo *> m_ServerInfo;
	// <serverid,gateid>
	std::unordered_map<int32, int32> m_GateServerInfo;
	// <mapid,serverid>
	std::unordered_map<int32, std::list<stServerInfo>> m_ServerMapInfo;
};
