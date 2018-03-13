#include "Client.h"
#include "ClientAuth.h"
#include "GameConnect.h"
#include "GateClientMgr.h"
#include "Config.h"
#include "GateCenterConnect.h"
#include "serverlog.h"
#include "objectpool.h"
#include "GlobalDefine.h"
#include "msgbase.h"

#include "MainType.h"
#include "ServerType.h"
#include "LoginType.h"
#include "Login.pb.h"
#include "ServerMsg.pb.h"

#define CLIENTAUTHINFO_ID_MAX 20000

static objectpool<ClientAuthInfo> &ClientAuthInfoPool()
{
	static objectpool<ClientAuthInfo> m(CLIENTAUTHINFO_ID_MAX, "ClientAuthInfo pools");
	return m;
}

static ClientAuthInfo *clientauthinfo_create()
{
	ClientAuthInfo *self = ClientAuthInfoPool().GetObject();
	if (!self)
	{
		RunStateError("创建 ClientAuthInfo 失败!");
		return NULL;
	}
	new(self) ClientAuthInfo();
	return self;
}

static void clientauthinfo_release(ClientAuthInfo *self)
{
	if (!self)
		return;
	self->~ClientAuthInfo();
	ClientAuthInfoPool().FreeObject(self);
}

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
			clientauthinfo_release(i.second);
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
	auto iter = m_ClientSecretInfo.find(msg.account());
	assert(iter == m_ClientSecretInfo.end());
	if (iter == m_ClientSecretInfo.end())
	{
		ClientAuthInfo *_pData = clientauthinfo_create();
		if (_pData)
		{
			_pData->ClientID = 0;
			_pData->GameServerID = msg.ngameid();
			_pData->Token = msg.account();
			_pData->Secret = msg.secret();
			_pData->Data.CopyFrom(msg.data());
			m_ClientSecretInfo.insert(std::make_pair(msg.account(), _pData));
		}
	}
}

void CClientAuth::KickClient(int32 clientid, bool closeclient, bool changeline)
{
	auto iter = m_ClientAuthInfo.find(clientid);
	assert(iter != m_ClientAuthInfo.end());
	if (iter != m_ClientAuthInfo.end())
	{
		// 通知玩家下线处理
		svrData::DelClient sendMsg;
		sendMsg.set_bchangeline(changeline);
		CGameConnect::Instance().SendMsgToServer(iter->second->GameServerID, sendMsg, SERVER_TYPE_MAIN, SVR_SUB_DEL_CLIENT, clientid);

		m_ClientSecretInfo.erase(iter->second->Token);
		clientauthinfo_release(iter->second);
		m_ClientAuthInfo.erase(iter);
		
		// 通知延迟关闭Client
		if (closeclient)
			CGateClientMgr::Instance().DelayCloseClient(clientid);
	}
}

void CClientAuth::QueryLogin(Msg *pMsg, CClient *cl)
{
	if (!pMsg || !cl)
		return;

	netData::Login msg;
	_CHECK_PARSE_(pMsg, msg);

	auto iter = m_ClientSecretInfo.find(msg.stoken());
	if (iter != m_ClientSecretInfo.end())
	{
		ClientAuthInfo *_pData = iter->second;
		if (!_pData->Secret.empty() &&_pData->Secret == msg.ssecret())
		{
			assert(_pData->ClientID == 0);
			_pData->ClientID = cl->GetClientID();
			m_ClientAuthInfo.insert(std::make_pair(cl->GetClientID(), _pData));
			ClientConnectLog("新的客户端认证成功！token:%s", msg.stoken().c_str());
			
			svrData::AddNewClient SendMsg;
			SendMsg.set_ngameid(cl->GetLogicServer());
			SendMsg.set_account(msg.stoken());
			CGateCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), SendMsg, SERVER_TYPE_MAIN, SVR_SUB_NEW_CLIENT, cl->GetClientID());
			cl->SetLogicServerID(_pData->GameServerID);
			cl->SetAlreadyAuth();
			return;
		}
	}

	netData::LoginRet sendMsg;
	sendMsg.set_ncode(netData::LoginRet::EC_FAIL);
	CGateClientMgr::Instance().SendMsg(cl, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_LOGIN_RET);
	// 认证失败
	ClientConnectError("新的客户端认证失败！token:%s", msg.stoken().c_str());
}

ClientAuthInfo *CClientAuth::FindAuthInfo(int32 clientid)
{
	auto iter = m_ClientAuthInfo.find(clientid);
	assert(iter != m_ClientAuthInfo.end());
	if (iter != m_ClientAuthInfo.end())
		return iter->second;

	return nullptr;
}