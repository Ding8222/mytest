/*
* 服务器负载管理
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>
#include <list>

struct ServerStatusInfo
{
	ServerStatusInfo()
	{
		nLineID = 0;
		nServerID = 0;
		nPort = 0;
		nMaxClient = 0;
		nNowClient = 0;
		IP.clear();
	}
	int32 nLineID;
	int32 nServerID;
	int32 nPort;
	int32 nMaxClient;
	int32 nNowClient;
	std::string IP;
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

struct stQueue
{
	int32 nCurrentIndex;
	int32 nMaxIndex;
	std::vector<ServerStatusInfo *> vGate;

	void clear()
	{
		nCurrentIndex = 0;
		nMaxIndex = 0;
		vGate.clear();
	}

	ServerStatusInfo *pop()
	{
		if (nMaxIndex > 0)
		{
			if (nCurrentIndex >= nMaxIndex)
				nCurrentIndex = 0;

			return vGate[nCurrentIndex++];
		}
		return nullptr;
	}

	void push(ServerStatusInfo *info)
	{
		vGate.push_back(info);
		nMaxIndex = vGate.size();
	}
};

#define ServerStatusMgr CServerStatusMgr::Instance()
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
	
	ServerStatusInfo *GetGameServerInfo(int32 mapid, int32 lineid = 0);
	ServerStatusInfo *GetGateServerInfo();
private:

	// 注册了的服务器的信息
	// <serverid,gateid>
	std::unordered_map<int32, ServerStatusInfo *> m_GameServerInfo;
	std::unordered_map<int32, ServerStatusInfo *> m_GateServerInfo;
	// <mapid,serverid>
	std::unordered_map<int32, std::list<stServerInfo>> m_ServerMapInfo;
	stQueue m_GateQueue;
};
