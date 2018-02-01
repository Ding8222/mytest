#include "stdfx.h"
#include "ServerStatusMgr.h"

CServerStatusMgr::CServerStatusMgr()
{
	m_ServerInfo.clear();
}

CServerStatusMgr::~CServerStatusMgr()
{
	for (auto &i: m_ServerInfo)
	{
		delete i.second;
	}
	m_ServerInfo.clear();
}

bool CServerStatusMgr::AddNewServer(ServerInfo *info)
{
	auto iter = m_ServerInfo.find(info->nServerID);
	assert(iter == m_ServerInfo.end());
#ifdef _DEBUG
	auto iterG = m_GateServerInfo.find(info->nGateID);
	assert(iterG == m_GateServerInfo.end());
#endif
	if (iter == m_ServerInfo.end())
	{
		m_ServerInfo[info->nServerID] = info;
		m_GateServerInfo[info->nGateID] = info;
		log_error("新的逻辑服注册到中心服务器：ID[%d]", info->nServerID);
		return true;
	}

	return false;
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
		m_GateServerInfo.erase(iter->second->nGateID);
		delete iter->second;
		m_ServerInfo.erase(iter);
	}
}

void CServerStatusMgr::DelServerByGateID(int id)
{
	auto iter = m_GateServerInfo.find(id);
	if (iter != m_GateServerInfo.end())
	{
		m_ServerInfo.erase(iter->second->nServerID);
		delete iter->second;
		m_GateServerInfo.erase(iter);
	}
}

ServerInfo *CServerStatusMgr::GetGateInfoByServerID(int id)
{
	auto iter = m_ServerInfo.find(id);
	if (iter != m_ServerInfo.end())
	{
		return iter->second;
	}
	return nullptr;
}