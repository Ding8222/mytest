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

bool CClientAuth::Init()
{
	m_ClientAuthInfo.resize(CLIENT_ID_MAX + 1, nullptr);
	return true;
}

void CClientAuth::Destroy()
{
	for (auto &i : m_ClientSecretInfo)
	{
		if (i.second)
		{
			clientauthinfo_release(i.second);
		}
	}

	m_ClientSecretInfo.clear();
	m_ClientAuthInfo.clear();
}

void CClientAuth::AsLogicServerDisconnect(int logicserverid)
{
	auto iterB = m_ClientSecretInfo.begin();
	for (; iterB != m_ClientSecretInfo.end(); )
	{
		ClientAuthInfo *info = iterB->second;
		if (info->GameServerID == logicserverid)
		{
			int32 &clientid = info->ClientID;
			if (clientid <= 0 || clientid >= (int32)m_ClientAuthInfo.size())
			{
				RunStateError("game连接断开时时Client的ID错误!");
				return;
			}

			m_ClientAuthInfo[clientid] = nullptr;
			clientauthinfo_release(info);
			iterB = m_ClientSecretInfo.erase(iterB);
		}
		else
			++iterB;
	}
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
	if (clientid <= 0 || clientid >= (int32)m_ClientAuthInfo.size())
	{
		RunStateError("更新GameSvrID时Client的ID错误!");
		return;
	}

	assert(m_ClientAuthInfo[clientid]);
	m_ClientAuthInfo[clientid]->GameServerID = gameid;
}

void CClientAuth::KickClient(int32 clientid, bool closeclient)
{
	if (clientid <= 0 || clientid >= (int32)m_ClientAuthInfo.size())
	{
		RunStateError("踢出Client的ID错误!");
		return;
	}

	ClientAuthInfo *info = m_ClientAuthInfo[clientid];
	assert(info);
	if (info)
	{
		// 通知玩家下线处理
		svrData::DelClient sendMsg;
		GameConnect.SendMsgToServer(info->GameServerID, sendMsg, SERVER_TYPE_MAIN, SVR_SUB_DEL_CLIENT, clientid);

		// 通知延迟关闭Client
		if (closeclient)
		{
			RunStateLog("玩家被踢下线！账号：%s", info->Account.c_str());
			GateClientMgr.DelayCloseClient(clientid);
		}
		else
			RunStateLog("玩家连接断开！账号：%s", info->Account.c_str());

		m_ClientSecretInfo.erase(info->Account);
		clientauthinfo_release(info);
		m_ClientAuthInfo[clientid] = nullptr;
	}
}

void CClientAuth::QueryLogin(Msg *pMsg, CClient *cl)
{
	if (!pMsg || !cl)
		return;

	netData::Login msg;
	_CHECK_PARSE_(pMsg, msg);

	netData::LoginRet sendMsg;

	int nClientID = cl->GetClientID();
	if (nClientID > 0 && nClientID < (int32)m_ClientAuthInfo.size())
	{
		auto iter = m_ClientSecretInfo.find(msg.account());
		if (iter != m_ClientSecretInfo.end())
		{
			ClientAuthInfo *_pData = iter->second;
			if (!_pData->Secret.empty() && _pData->Secret == msg.secret())
			{
				assert(_pData->ClientID == 0);
				_pData->ClientID = nClientID;
				m_ClientAuthInfo[nClientID] = _pData;
				_pData->Data.set_bchangeline(false);
				GameConnect.SendMsgToServer(_pData->GameServerID, _pData->Data, SERVER_TYPE_MAIN, SVR_SUB_PLAYERDATA, nClientID);
				cl->SetAlreadyAuth();
				return;
			}
			else
			{
				sendMsg.set_ncode(netData::LoginRet::EC_SECRET);
				ClientConnectError("登陆失败！account:%s，秘钥：[%s]错误！存储秘钥：[%s]", msg.account().c_str(), msg.secret().c_str(), _pData->Secret.c_str());
			}
		}
		else
		{
			sendMsg.set_ncode(netData::LoginRet::EC_ACCOUNT);
			ClientConnectError("登陆失败！account:%s，没有找到账号信息", msg.account().c_str());
		}
	}
	else
		RunStateError("请求登陆时Client的ID错误!");

	GateClientMgr.SendMsg(cl, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_LOGIN_RET);
}

ClientAuthInfo *CClientAuth::FindAuthInfo(int32 clientid)
{
	if (clientid <= 0 || clientid >= (int32)m_ClientAuthInfo.size())
	{
		RunStateError("要超找的Client的ID错误!");
		return nullptr;
	}

	assert(m_ClientAuthInfo[clientid]);
	return m_ClientAuthInfo[clientid];
}