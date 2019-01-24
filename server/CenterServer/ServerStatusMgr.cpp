#include "serverinfo.h"
#include "GlobalDefine.h"
#include "ServerStatusMgr.h"
#include "serverlog.h"
#include "objectpool.h"
#include "msgbase.h"
#include "CenterPlayerMgr.h"
#include "ClientAuthMgr.h"

#include "ServerMsg.pb.h"

#define SERVERSTATUSINFO_ID_MAX 100

static objectpool<ServerStatusInfo> &ServerStatusInfoPool()
{
	static objectpool<ServerStatusInfo> m(SERVERSTATUSINFO_ID_MAX, "ServerStatusInfo pools");
	return m;
}

static ServerStatusInfo *serverstatusinfo_create()
{
	ServerStatusInfo *self = ServerStatusInfoPool().GetObject();
	if (!self)
	{
		RunStateError("创建 ServerStatusInfo 失败!");
		return NULL;
	}
	new(self) ServerStatusInfo();
	return self;
}

static void serverstatusinfo_release(ServerStatusInfo *self)
{
	if (!self)
		return;
	self->~ServerStatusInfo();
	ServerStatusInfoPool().FreeObject(self);
}

CServerStatusMgr::CServerStatusMgr()
{
	m_GameServerInfo.clear();
	m_GateServerInfo.clear();
	m_ServerMapInfo.clear();
	m_GateQueue.clear();
}

CServerStatusMgr::~CServerStatusMgr()
{
	Destroy();
}

void CServerStatusMgr::Destroy()
{
	for (auto &i : m_GameServerInfo)
	{
		serverstatusinfo_release(i.second);
	}
	m_GameServerInfo.clear();

	for (auto &i : m_GateServerInfo)
	{
		serverstatusinfo_release(i.second);
	}
	m_GateServerInfo.clear();

	for (auto &i : m_ServerMapInfo)
	{
		i.second->clear();
		delete i.second;
	}
	m_ServerMapInfo.clear();
	m_GateQueue.clear();
}

void CServerStatusMgr::AddGameServer(serverinfo *info, Msg *pMsg)
{
	svrData::ServerLoadInfo msg;
	_CHECK_PARSE_(pMsg, msg);

	int32 nServerID = info->GetServerID();
	auto iter = m_GameServerInfo.find(nServerID);
	assert(iter == m_GameServerInfo.end());
	if (iter == m_GameServerInfo.end())
	{
		ServerStatusInfo *_pInfo = serverstatusinfo_create();
		if (_pInfo)
		{
			_pInfo->nServerID = nServerID;
			_pInfo->nMaxClient = msg.nmaxclient();
			_pInfo->nNowClient = msg.nnowclient();
			_pInfo->IP = msg.sip();
			_pInfo->nPort = msg.nport();

			m_GameServerInfo[_pInfo->nServerID] = _pInfo;
			RunStateLog("服务器[%s]注册到服务器状态管理器：ID[%d]", info->GetServerName(), _pInfo->nServerID);
		}
	}

	for (auto &i : msg.mapid())
	{
		auto iter = m_ServerMapInfo.find(i);
		if (iter != m_ServerMapInfo.end())
		{
			stQueue<ServerStatusInfo> *queue = iter->second;

			auto iter2 = m_GameServerInfo.find(nServerID);
			assert(iter2 != m_GameServerInfo.end());
			if (iter2 != m_GameServerInfo.end())
			{
				queue->add(iter2->second);
			}
		}
		else
		{
			auto iter2 = m_GameServerInfo.find(nServerID);
			assert(iter2 != m_GameServerInfo.end());
			if (iter2 != m_GameServerInfo.end())
			{
				stQueue<ServerStatusInfo> *queue = new stQueue<ServerStatusInfo>();
				queue->add(iter2->second);

				m_ServerMapInfo.insert(std::make_pair(i, queue));
			}
		}
	}

	for (auto &i : msg.info())
	{
		CenterPlayerMgr.AddPlayer(i.nguid(), i.account(), i.nclientid(), i.ngameid(), nServerID, i.ngateid());
		ClientAuthMgr.SetPlayerOnline(i.account(), i.nguid());
	}
}

void CServerStatusMgr::AddGateServer(serverinfo *info, Msg *pMsg)
{
	svrData::ServerLoadInfo msg;
	_CHECK_PARSE_(pMsg, msg);

	int32 nServerID = info->GetServerID();
	auto iter = m_GateServerInfo.find(nServerID);
	assert(iter == m_GateServerInfo.end());
	if (iter == m_GateServerInfo.end())
	{
		ServerStatusInfo *_pInfo = serverstatusinfo_create();
		if (_pInfo)
		{
			_pInfo->nServerID = nServerID;
			_pInfo->nMaxClient = msg.nmaxclient();
			_pInfo->nNowClient = msg.nnowclient();
			_pInfo->IP = msg.sip();
			_pInfo->nPort = msg.nport();

			m_GateServerInfo[_pInfo->nServerID] = _pInfo;
			m_GateQueue.add(_pInfo);
			RunStateLog("服务器[%s]注册到服务器状态管理器：ID[%d]", info->GetServerName(), _pInfo->nServerID);
		}
	}
}

void CServerStatusMgr::UpdateGameServerLoad(int32 id, int32 clientcountnow, int32 clientcountmax)
{
	auto iter = m_GameServerInfo.find(id);
	if (iter != m_GameServerInfo.end())
	{
		iter->second->nNowClient = clientcountnow;
		iter->second->nMaxClient = clientcountmax;
	}
}

void CServerStatusMgr::UpdateGateServerLoad(int32 id, int32 clientcountnow, int32 clientcountmax)
{
	auto iter = m_GateServerInfo.find(id);
	if (iter != m_GateServerInfo.end())
	{
		iter->second->nNowClient = clientcountnow;
		iter->second->nMaxClient = clientcountmax;
	}
}

void CServerStatusMgr::DelGameServer(int32 serverid)
{
	auto iter = m_GameServerInfo.find(serverid);
	if (iter != m_GameServerInfo.end())
	{
		for (auto &i : m_ServerMapInfo)
		{
			i.second->del(iter->second);
		}
		serverstatusinfo_release(iter->second);
		m_GameServerInfo.erase(iter);
	}
}

void CServerStatusMgr::DelGateServer(int32 serverid)
{
	auto iter = m_GateServerInfo.find(serverid);
	if (iter != m_GateServerInfo.end())
	{
		m_GateQueue.del(iter->second);
		serverstatusinfo_release(iter->second);
		m_GateServerInfo.erase(iter);
	}
}

ServerStatusInfo *CServerStatusMgr::GetGameServerInfo(int32 mapid, int32 lineid)
{
	auto iter = m_ServerMapInfo.find(mapid);
	if (iter != m_ServerMapInfo.end())
	{
		stQueue<ServerStatusInfo> *serverqueue = iter->second;
		if (serverqueue->size() > 0)
		{
			//这里列出了所有可进入的服务器ID
			//可以根据负载选择一个服务器信息返回
			ServerStatusInfo *info = nullptr;
			for (size_t i = 0; i < serverqueue->size(); ++i)
			{
				info = serverqueue->pop();
				if (info)
				{
					if (lineid == 0 || info->nLineID == lineid)
					{
						return info;
					}
				}
			}
		}
	}
	return nullptr;
}

ServerStatusInfo *CServerStatusMgr::GetGateServerInfo()
{
	return m_GateQueue.pop();
}
