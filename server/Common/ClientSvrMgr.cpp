
#include <unordered_map>
#include "msgbase.h"
#include "ClientSvrMgr.h"
#include "GlobalDefine.h"

CClientSvrMgr::CClientSvrMgr()
{
	m_ClientSvr.clear();
}

CClientSvrMgr::~CClientSvrMgr()
{
	m_ClientSvr.clear();
}

void CClientSvrMgr::AddClientSvr(int64 clientid, int serverid, int servertype)
{
	auto iter = m_ClientSvr.find(clientid);
	if (iter != m_ClientSvr.end())
	{
		ClientSvr &temp = iter->second;
		temp.nClientID = clientid;
		switch (servertype)
		{
		case ServerEnum::EST_LOGIN:
		{
			temp.nLoginServerID = serverid;
			break;
		}
		case ServerEnum::EST_GAME:
		{
			temp.nGameServerID = serverid;
			break;
		}
		case ServerEnum::EST_GATE:
		{
			temp.nGateID = serverid;
			break;
		}
		}
	}
	else
	{
		ClientSvr temp;
		temp.nClientID = clientid;
		switch (servertype)
		{
		case ServerEnum::EST_LOGIN:
		{
			temp.nLoginServerID = serverid;
			break;
		}
		case ServerEnum::EST_GAME:
		{
			temp.nGameServerID = serverid;
			break;
		}
		case ServerEnum::EST_GATE:
		{
			temp.nGateID = serverid;
			break;
		}
		}
		m_ClientSvr.insert(std::make_pair(temp.nClientID, temp));
	}
}

void CClientSvrMgr::DelClientSvr(int64 id)
{
	auto iter = m_ClientSvr.find(id);
	assert(iter != m_ClientSvr.end());
	if (iter != m_ClientSvr.end())
	{
		m_ClientSvr.erase(iter);
	}
}

ClientSvr *CClientSvrMgr::GetClientSvr(int64 id)
{
	if (id > 0)
	{
		auto iter = m_ClientSvr.find(id);
		if (iter != m_ClientSvr.end())
		{
			return &(iter->second);
		}
	}

	return nullptr;
}