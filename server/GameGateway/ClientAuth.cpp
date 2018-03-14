﻿#include "Client.h"
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

void CClientAuth::AddAccountInfo(Msg *pMsg)
{
	if (!pMsg)
		return;

	svrData::ClientAccount msg;
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
			_pData->Account = msg.account();
			_pData->Secret = msg.secret();
			_pData->Data.CopyFrom(msg.data());
			m_ClientSecretInfo.insert(std::make_pair(msg.account(), _pData));
		}
	}
	else
		RunStateError("重复添加账号：%s登陆信息！", msg.account().c_str());
}

void CClientAuth::UpdateGameSvrID(int32 clientid, int32 gameid)
{
	auto iter = m_ClientAuthInfo.find(clientid);
	assert(iter != m_ClientAuthInfo.end());
	if (iter != m_ClientAuthInfo.end())
	{
		iter->second->GameServerID = gameid;
	}
}

void CClientAuth::KickClient(int32 clientid, bool closeclient)
{
	auto iter = m_ClientAuthInfo.find(clientid);
	assert(iter != m_ClientAuthInfo.end());
	if (iter != m_ClientAuthInfo.end())
	{
		// 通知玩家下线处理
		svrData::DelClient sendMsg;
		CGameConnect::Instance().SendMsgToServer(iter->second->GameServerID, sendMsg, SERVER_TYPE_MAIN, SVR_SUB_DEL_CLIENT, clientid);

		m_ClientSecretInfo.erase(iter->second->Account);
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

	netData::LoginRet sendMsg;

	auto iter = m_ClientSecretInfo.find(msg.account());
	if (iter != m_ClientSecretInfo.end())
	{
		ClientAuthInfo *_pData = iter->second;
		if (!_pData->Secret.empty() &&_pData->Secret == msg.secret())
		{
			assert(_pData->ClientID == 0);
			_pData->ClientID = cl->GetClientID();
			m_ClientAuthInfo.insert(std::make_pair(cl->GetClientID(), _pData));
			
			svrData::AddNewClient SendMsg;
			SendMsg.set_ngameid(cl->GetLogicServer());
			SendMsg.set_account(msg.account());
			CGateCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), SendMsg, SERVER_TYPE_MAIN, SVR_SUB_NEW_CLIENT, cl->GetClientID());
			cl->SetAlreadyAuth();
			return;
		}
		else
		{
			sendMsg.set_ncode(netData::LoginRet::EC_SECRET);
			ClientConnectError("登陆失败！account:%s，秘钥：%s错误！", msg.account().c_str(), msg.secret().c_str());
		}
	}
	else
	{
		sendMsg.set_ncode(netData::LoginRet::EC_ACCOUNT);
		ClientConnectError("登陆失败！account:%s，没有找到账号信息", msg.account().c_str());
	}

	CGateClientMgr::Instance().SendMsg(cl, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_LOGIN_RET);
}

ClientAuthInfo *CClientAuth::FindAuthInfo(int32 clientid)
{
	auto iter = m_ClientAuthInfo.find(clientid);
	assert(iter != m_ClientAuthInfo.end());
	if (iter != m_ClientAuthInfo.end())
		return iter->second;

	return nullptr;
}