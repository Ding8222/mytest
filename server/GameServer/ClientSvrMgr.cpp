#include"stdfx.h"
#include "ClientSvrMgr.h"
#include "GameCenterConnect.h"
#include "Config.h"

CClientSvrMgr::CClientSvrMgr()
{
	m_ClientSvrInfo.clear();
}

CClientSvrMgr::~CClientSvrMgr()
{
	m_ClientSvrInfo.clear();
}

void CClientSvrMgr::AddClientSvr(int64 clientid, int serverid)
{
	auto iter = m_ClientSvrInfo.find(clientid);
	assert(iter == m_ClientSvrInfo.end());
	if (iter == m_ClientSvrInfo.end())
	{
		m_ClientSvrInfo.insert(std::make_pair(clientid, serverid));
	}
}

void CClientSvrMgr::DelClientSvr(int64 clientid)
{
	auto iter = m_ClientSvrInfo.find(clientid);
	assert(iter != m_ClientSvrInfo.end());
	if (iter != m_ClientSvrInfo.end())
		m_ClientSvrInfo.erase(clientid);
}

void CClientSvrMgr::DelAllClientSvr()
{
	m_ClientSvrInfo.clear();
}

int CClientSvrMgr::FindClientSvr(int64 clientid)
{
	auto iter = m_ClientSvrInfo.find(clientid);
	assert(iter != m_ClientSvrInfo.end());
	if (iter != m_ClientSvrInfo.end())
		return iter->second;
	return 0;
}
