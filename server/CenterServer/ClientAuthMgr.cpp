#include "ClientAuthMgr.h"
#include "CentServerMgr.h"
#include "ServerStatusMgr.h"

#include "MainType.h"
#include "ServerType.h"
#include "LoginType.h"
#include "Login.pb.h"
#include "ServerMsg.pb.h"

CClientAuthMgr::CClientAuthMgr()
{
	m_ClientInfo.clear();
}

CClientAuthMgr::~CClientAuthMgr()
{
	Destroy();
}

void CClientAuthMgr::Destroy()
{
	m_ClientInfo.clear();
}

// 添加认证信息
void CClientAuthMgr::AddClientAuthInfo(Msg *pMsg, int64 clientid)
{
	netData::Auth msg;
	_CHECK_PARSE_(pMsg, msg);
	
	ClientAuthInfo Info;
	Info.Token = msg.setoken();
	Info.Secret = msg.ssecret();
	m_ClientInfo.insert(std::make_pair(clientid, Info));

	CCentServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_DB, clientid);
	return ;
}

// 移除认证信息
void CClientAuthMgr::DelClientAuthInfo(int64 clientid)
{
	auto iter = m_ClientInfo.find(clientid);
	assert(iter != m_ClientInfo.end());
	if(iter != m_ClientInfo.end())
		m_ClientInfo.erase(iter);
}

// 发送认证信息到逻辑服
void CClientAuthMgr::SendAuthInfoToLogic(Msg *pMsg, int64 clientid)
{
	netData::AuthRet msg;
	_CHECK_PARSE_(pMsg, msg);

	auto iter = m_ClientInfo.find(clientid);
	if(iter!= m_ClientInfo.end())
	{
		ClientAuthInfo &Info = iter->second;

		ServerStatusInfo *_pInfo = CServerStatusMgr::Instance().GetGateInfoByServerID(msg.nserverid());
		if (_pInfo)
		{
			msg.set_ncode(netData::AuthRet::EC_SUCC);
			msg.set_nserverid(_pInfo->nServerID);
			msg.set_ip(_pInfo->chIP);
			msg.set_port(_pInfo->nPort);

			svrData::ClientToken sendMsg;
			sendMsg.set_setoken(Info.Token);
			sendMsg.set_ssecret(Info.Secret);
			CCentServerMgr::Instance().SendMsgToServer(sendMsg, SERVER_TYPE_MAIN, SVR_SUB_CLIENT_TOKEN, ServerEnum::EST_GATE, 0, _pInfo->nServerID);

		}
		else
			msg.set_ncode(netData::AuthRet::EC_SERVER);
	}
	else
		msg.set_ncode(netData::AuthRet::EC_AUTHINFO);

	CCentServerMgr::Instance().SendMsgToServer(msg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET, ServerEnum::EST_LOGIN, clientid);
}

void CClientAuthMgr::SendLoadPlayerDataToLogic(Msg *pMsg, int64 clientid)
{
	CCentServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_GAME, clientid);
}