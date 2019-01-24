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

template <typename T>
class stQueue
{
public:
	stQueue()
	{
		nCurrentIndex = 0;
		nMaxIndex = 0;
		vQueue.clear();
	}
	
	void clear()
	{
		nCurrentIndex = 0;
		nMaxIndex = 0;
		vQueue.clear();
	}

	T *pop()
	{
		if (nMaxIndex > 0)
		{
			if (nCurrentIndex >= nMaxIndex)
				nCurrentIndex = 0;

			return vQueue[nCurrentIndex++];
		}
		return nullptr;
	}

	void add(T *info)
	{
		if (info == nullptr)
			return;

		vQueue.push_back(info);
		nMaxIndex = vQueue.size();
	}

	void del(T *info)
	{
		if (info == nullptr)
			return;

		std::vector<T *>::iterator _Iter
			= std::find_if(vQueue.begin(), vQueue.end(), [&](T* q)->bool { return q->nServerID == info->nServerID; });
		if (_Iter != vQueue.end())
			vQueue.erase(_Iter);
		nMaxIndex = vQueue.size();
	}

	size_t size()
	{
		return vQueue.size();
	}

private:
	int32 nCurrentIndex;
	int32 nMaxIndex;
	std::vector<T *> vQueue;
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
	std::unordered_map<int32, stQueue<ServerStatusInfo> *> m_ServerMapInfo;
	stQueue<ServerStatusInfo> m_GateQueue;
};
