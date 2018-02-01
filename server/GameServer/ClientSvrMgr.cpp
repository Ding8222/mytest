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
		svrData::UpdateServerLoad sendMsg;
		sendMsg.set_nclientcountmax(2000);
		sendMsg.set_nclientcountnow(m_ClientSvrInfo.size());
		CGameCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), sendMsg, SERVER_TYPE_MAIN, SVR_SUB_UPDATE_LOAD);

	}
}

void CClientSvrMgr::DelClientSvr(int64 clientid)
{
	auto iter = m_ClientSvrInfo.find(clientid);
	assert(iter != m_ClientSvrInfo.end());
	if (iter != m_ClientSvrInfo.end())
		m_ClientSvrInfo.erase(clientid);
}

int CClientSvrMgr::FindClientSvr(int64 clientid)
{
	auto iter = m_ClientSvrInfo.find(clientid);
	assert(iter != m_ClientSvrInfo.end());
	if (iter != m_ClientSvrInfo.end())
		return iter->second;
	return 0;
}
