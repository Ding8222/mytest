#include "serverinfo.h"
#include "ServerStatusMgr.h"
#include "serverlog.h"
#include "objectpool.h"
#include "msgbase.h"

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
	m_ServerInfo.clear();
	m_GateServerInfo.clear();
}

CServerStatusMgr::~CServerStatusMgr()
{
	Destroy();
}

void CServerStatusMgr::Destroy()
{
	for (auto &i : m_ServerInfo)
	{
		serverstatusinfo_release(i.second);
	}
	m_ServerInfo.clear();
	m_GateServerInfo.clear();
}

void CServerStatusMgr::AddNewServer(serverinfo *info, Msg *pMsg)
{
	svrData::ServerLoadInfo msg;
	_CHECK_PARSE_(pMsg, msg);
	
	auto iter = m_ServerInfo.find(info->GetServerID());
	assert(iter == m_ServerInfo.end());
	if (iter == m_ServerInfo.end())
	{
		ServerStatusInfo *_pInfo = serverstatusinfo_create();
		if (_pInfo)
		{
			_pInfo->nServerID = info->GetServerID();
			_pInfo->nServerType = info->GetServerType();
			_pInfo->nMaxClient = msg.nmaxclient();
			_pInfo->nNowClient = msg.nnowclient();
			strncpy_s(_pInfo->chIP, MAX_IP_LEN, msg.sip().c_str(), msg.sip().size());
			_pInfo->nPort = msg.nport();
			_pInfo->nSubServerID = msg.nsubserverid();

			m_ServerInfo[_pInfo->nServerID] = _pInfo;
			RunStateLog("新的服务器注册到服务器状态管理器：ID[%d]，类型：[%d]", _pInfo->nServerID, _pInfo->nServerType);
			if (_pInfo->nSubServerID > 0)
			{
				m_GateServerInfo[_pInfo->nSubServerID] = _pInfo->nServerID;
			}
		}
	}
}

void CServerStatusMgr::UpdateServerLoad(int id, int clientcountnow, int clientcountmax)
{
	auto iter = m_ServerInfo.find(id);
	if (iter != m_ServerInfo.end())
	{
		iter->second->nNowClient = clientcountnow;
		iter->second->nMaxClient = clientcountmax;
	}
}

void CServerStatusMgr::DelServerID(int serverid)
{
	auto iter = m_ServerInfo.find(serverid);
	if (iter != m_ServerInfo.end())
	{
		ServerStatusInfo *info = iter->second;
		if (info->nSubServerID > 0)
		{
#ifdef _DEBUG
			auto iterF = m_GateServerInfo.find(info->nSubServerID);
			assert(iterF != m_GateServerInfo.end());
#endif // _DEBUG
			m_GateServerInfo.erase(info->nSubServerID);
		}
		serverstatusinfo_release(info);
		m_ServerInfo.erase(iter);
	}
}

ServerStatusInfo *CServerStatusMgr::GetGateInfoByServerID(int id)
{
	auto iterGame = m_ServerInfo.find(id);
	if (iterGame != m_ServerInfo.end())
	{
		auto iterGate = m_GateServerInfo.find(id);
		if (iterGate != m_GateServerInfo.end())
		{
			auto iterRet = m_ServerInfo.find(iterGate->second);
			if (iterRet != m_ServerInfo.end())
			{
				return iterRet->second;
			}
		}
	}
	return nullptr;
}