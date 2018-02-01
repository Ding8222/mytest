#include "stdfx.h"
#include "Client.h"
#include "ClientAuth.h"
#include "GameConnect.h"
#include "GateClientMgr.h"
#include "Config.h"

#include "Login.pb.h"

CClientAuth::CClientAuth()
{
	m_ClientSecretInfo.clear();
	m_ClientAuthInfo.clear();
}

CClientAuth::~CClientAuth()
{
	Destroy();
}

void CClientAuth::Destroy()
{
	for (auto &i : m_ClientSecretInfo)
	{
		if (i.second)
		{
			delete i.second;
			i.second = nullptr;
		}
	}

	m_ClientSecretInfo.clear();
	m_ClientAuthInfo.clear();
}

void CClientAuth::AddAuthInfo(Msg *pMsg)
{
	if (!pMsg)
		return;

	svrData::ClientToken msg;
	_CHECK_PARSE_(pMsg, msg);

	auto iter = m_ClientSecretInfo.find(msg.setoken());
	if (iter == m_ClientSecretInfo.end())
	{
		ClientAuthInfo *_pData = new ClientAuthInfo;
		_pData->ClientID = 0;
		_pData->Token = msg.setoken();
		_pData->Secret = msg.ssecret();
		m_ClientSecretInfo.insert(std::make_pair(msg.setoken(), _pData));
	}
	else
	{
		// T下线并更新 ClientAuthInfo
		ClientAuthInfo *_pData = iter->second;
		KickClient(_pData->ClientID);
		_pData->ClientID = 0;
		_pData->Token = msg.setoken();
		_pData->Secret = msg.ssecret();
	}
}

void CClientAuth::KickClient(int64 clientid)
{
	m_ClientAuthInfo.erase(clientid);
	// 通知延迟关闭Client
	CGateClientMgr::Instance().DelayCloseClient(clientid);
	// 通知玩家下线处理
}

void CClientAuth::AddNewClient(Msg *pMsg, CClient *cl)
{
	if (!pMsg || !cl)
		return;

	netData::Login msg;
	_CHECK_PARSE_(pMsg, msg);

	auto iter = m_ClientSecretInfo.find(msg.stoken());
	if (iter != m_ClientSecretInfo.end())
	{
		ClientAuthInfo *_pData = iter->second;
		if (_pData->ClientID > 0)
		{
			KickClient(_pData->ClientID);
		}

		_pData->ClientID = cl->GetClientID();
		m_ClientAuthInfo.insert(std::make_pair(cl->GetClientID(), _pData));
		// 认证成功
		cl->SetAlreadyAuth();
		log_error("新的客户端认证成功！token:%s", msg.stoken().c_str());
		svrData::AddNewClient sendMsg;

		CGameConnect::Instance().SendMsgToServer(CConfig::Instance().GetGameServerID(), sendMsg, SERVER_TYPE_MAIN, SVR_SUB_NEW_CLIENT, cl->GetClientID());
		return ;
	}

	// 认证失败
	log_error("新的客户端认证失败！token:%s", msg.stoken().c_str());
	return ;
}

void CClientAuth::Offline(int64 clientid)
{
	auto iter = m_ClientAuthInfo.find(clientid);
	if (iter != m_ClientAuthInfo.end())
	{
		m_ClientSecretInfo.erase(iter->second->Token);
		delete iter->second;
	}
	KickClient(clientid);
}