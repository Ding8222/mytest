#include "serverinfo.h"
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
	m_ServerMapInfo.clear();
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
			strncpy_s(_pInfo->chIP, MAX_IP_LEN, msg.sip().c_str(), msg.sip().size());
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
			std::list<stServerInfo> &maplist = iter->second;

			stServerInfo temp;
			temp.nLineiD = msg.nlineid();
			temp.nServerID = nServerID;

			maplist.push_back(temp);
		}
		else
		{
			stServerInfo temp;
			temp.nLineiD = msg.nlineid();
			temp.nServerID = nServerID;

			std::list<stServerInfo> maplist;
			maplist.push_back(temp);

			m_ServerMapInfo.insert(std::make_pair(i, maplist));
		}
	}

	for (auto &i : msg.info())
	{
		CCenterPlayerMgr::Instance().AddPlayer(i.nguid(), i.account(), i.nclientid(), nServerID, i.ngateid());
		CClientAuthMgr::Instance().SetPlayerOnline(i.account(), i.nguid());
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
			strncpy_s(_pInfo->chIP, MAX_IP_LEN, msg.sip().c_str(), msg.sip().size());
			_pInfo->nPort = msg.nport();

			m_GateServerInfo[_pInfo->nServerID] = _pInfo;
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
		serverstatusinfo_release(iter->second);
		m_GameServerInfo.erase(iter);
	}
}

void CServerStatusMgr::DelGateServer(int32 serverid)
{
	auto iter = m_GateServerInfo.find(serverid);
	if (iter != m_GateServerInfo.end())
	{
		serverstatusinfo_release(iter->second);
		m_GateServerInfo.erase(iter);
	}
}

ServerStatusInfo *CServerStatusMgr::GetGameServerInfo(int32 id, int32 lineid)
{
	auto iter = m_ServerMapInfo.find(id);
	if (iter != m_ServerMapInfo.end())
	{
		std::list<stServerInfo> &serverset = iter->second;
		if (serverset.size() > 0)
		{
			//这里列出了所有可进入的服务器ID
			//可以根据负载选择一个服务器信息返回
			for (auto &i : serverset)
			{
				if (lineid == 0 || i.nLineiD == lineid)
				{
					auto iterG = m_GameServerInfo.find(i.nServerID);
					if (iterG != m_GameServerInfo.end())
					{
						return iterG->second;
					}
				}
			}
		}
	}
	return nullptr;
}

ServerStatusInfo *CServerStatusMgr::GetGateServerInfo()
{
	for (auto &i : m_GateServerInfo)
	{
		return i.second;
	}

	return nullptr;
}
