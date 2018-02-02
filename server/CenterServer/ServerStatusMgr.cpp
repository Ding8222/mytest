#include "stdfx.h"
#include "serverinfo.h"
#include "ServerStatusMgr.h"

CServerStatusMgr::CServerStatusMgr()
{
	m_ServerInfo.clear();
	m_GateServerInfo.clear();
}

CServerStatusMgr::~CServerStatusMgr()
{
	for (auto &i: m_ServerInfo)
	{
		delete i.second;
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
		ServerStatusInfo *_pInfo = new ServerStatusInfo;

		_pInfo->nServerID = info->GetServerID();
		_pInfo->nServerType = info->GetServerType();
		_pInfo->nMaxClient = msg.nmaxclient();
		_pInfo->nNowClient = msg.nnowclient();
		strncpy_s(_pInfo->chIP, MAX_IP_LEN, msg.sip().c_str(), msg.sip().size());
		_pInfo->nPort = msg.nport();
		_pInfo->nSubServerID = msg.nsubserverid();

		m_ServerInfo[_pInfo->nServerID] = _pInfo;
		log_error("新的服务器注册到服务器状态管理器：ID[%d]，类型：[%d]", _pInfo->nServerID, _pInfo->nServerType);
		if (_pInfo->nSubServerID > 0)
		{
			auto iterF = m_GateServerInfo.find(_pInfo->nSubServerID);
			if (iterF == m_GateServerInfo.end())
			{
				m_GateServerInfo[_pInfo->nSubServerID] = _pInfo->nServerID;
				return ;
			}
			return ;
		}
		return ;
	}

	return ;
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

void CServerStatusMgr::DelServerByGameID(int id)
{
	auto iter = m_ServerInfo.find(id);
	if (iter != m_ServerInfo.end())
	{
		if (iter->second->nSubServerID > 0)
		{
#ifdef _DEBUG
			auto iterF = m_GateServerInfo.find(iter->second->nSubServerID);
			assert(iterF != m_GateServerInfo.end());
#endif // _DEBUG
			m_GateServerInfo.erase(iter->second->nSubServerID);
		}
		delete iter->second;
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