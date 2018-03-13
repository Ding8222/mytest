#include "idmgr.c"
#include "ClientAuthMgr.h"
#include "CentServerMgr.h"
#include "ServerStatusMgr.h"
#include "objectpool.h"
#include "ServerLog.h"
#include "Config.h"
#include "ClientSvrMgr.h"

#include "MainType.h"
#include "ServerType.h"
#include "LoginType.h"
#include "Login.pb.h"
#include "ServerMsg.pb.h"

static objectpool<ClientAuthInfo> &ClientAuthInfoPool()
{
	static objectpool<ClientAuthInfo> m(CLIENT_ID_MAX, "ClientAuthInfo pools");
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

CClientAuthMgr::CClientAuthMgr()
{
	m_ClientInfoSet.clear();
}

CClientAuthMgr::~CClientAuthMgr()
{
	Destroy();
}

bool CClientAuthMgr::Init()
{
	m_ClientInfoSet.resize(CLIENT_ID_MAX + 1, nullptr);
	return true;
}

void CClientAuthMgr::Run()
{

}

void CClientAuthMgr::Destroy(bool bLoginDisconnect)
{
	for (size_t i = 0; i < m_ClientInfoSet.size(); ++i)
	{
		DelClientAuthInfo(i);
	}

	if(!bLoginDisconnect)
		m_ClientInfoSet.clear();
}

// 添加认证信息
void CClientAuthMgr::QueryAuth(Msg *pMsg, int32 clientid, int32 serverid)
{
	netData::Auth msg;
	_CHECK_PARSE_(pMsg, msg);

	bool bWaitKick = false;
	auto iter = m_PlayerOnlineMap.find(msg.setoken());
	if (iter != m_PlayerOnlineMap.end())
	{
		if (iter->second > 0)
		{
			//玩家在线，T下线
			svrData::KickClient SendMsg;
			CCentServerMgr::Instance().SendMsgToServer(SendMsg, SERVER_TYPE_MAIN, SVR_SUB_KICKCLIENT, ServerEnum::EST_GATE, iter->second);
			bWaitKick = true;
		}
		else
		{
			//正在认证中
			netData::AuthRet SendMsg;
			SendMsg.set_ncode(netData::AuthRet::EC_AUTHING);
			CCentServerMgr::Instance().SendMsgToServer(SendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET, ServerEnum::EST_LOGIN, clientid, serverid);
			return;
		}
	}

	ClientAuthInfo *newinfo = clientauthinfo_create();
	if (!newinfo)
	{
		log_error("创建ClientAuthInfo失败!");
		return;
	}

	newinfo->nLoginSvrID = serverid;
	newinfo->Token = msg.setoken();
	newinfo->Secret = msg.ssecret();
	m_ClientInfoSet[clientid] = newinfo;

	if(bWaitKick)
		m_PlayerLoginMap[msg.setoken()] = clientid;
	else
	{
		m_PlayerOnlineMap[msg.setoken()] = 0;
		CCentServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_DB, clientid, CConfig::Instance().GetDBID());
	}
	return ;
}

// 移除认证信息
void CClientAuthMgr::DelClientAuthInfo(int32 clientid)
{
	if (clientid <= 0 || clientid >= static_cast<int>(m_ClientInfoSet.size()))
	{
		log_error("要释放的ClientAuthInfo的ID错误!");
		return;
	}
	
	clientauthinfo_release(m_ClientInfoSet[clientid]);
	m_ClientInfoSet[clientid] = nullptr;
}

ClientAuthInfo *CClientAuthMgr::FindClientAuthInfo(int32 clientid)
{
	if (clientid <= 0 || clientid >= static_cast<int>(m_ClientInfoSet.size()))
	{
		RunStateError("要查找的ClientAuthInfo的ID错误!");
		return nullptr;
	}

	assert(m_ClientInfoSet[clientid]);
	return m_ClientInfoSet[clientid];
}

int32 CClientAuthMgr::GetClientLoginSvr(int32 clientid)
{
	if (clientid <= 0 || clientid >= static_cast<int>(m_ClientInfoSet.size()))
	{
		RunStateError("要查找的ClientAuthInfo的ID错误!");
		return 0;
	}

	assert(m_ClientInfoSet[clientid]);
	return m_ClientInfoSet[clientid]->nLoginSvrID;
}

void CClientAuthMgr::SetCenterClientID(const std::string &token, int32 id)
{
	m_PlayerOnlineMap[token] = id;
// 	auto iter = m_PlayerOnlineMap.find(token);
// 	assert(iter != m_PlayerOnlineMap.end());
// 	if (iter != m_PlayerOnlineMap.end())
// 	{
// 		iter->second = id;
// 	}
}

void CClientAuthMgr::ClientOffline(const std::string &token)
{
	auto iter = m_PlayerOnlineMap.find(token);
	assert(iter != m_PlayerOnlineMap.end());
	if (iter != m_PlayerOnlineMap.end())
	{
		auto iterF = m_PlayerLoginMap.find(token);
		if (iterF != m_PlayerLoginMap.end())
		{
			assert(iterF->second);
			m_PlayerOnlineMap[token] = 0;
			netData::Auth SendMsg;
			SendMsg.set_setoken(token);
			SendMsg.set_ssecret(token);
			CCentServerMgr::Instance().SendMsgToServer(SendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH, ServerEnum::EST_DB, iterF->second, CConfig::Instance().GetDBID());
			m_PlayerLoginMap.erase(iterF);
		}
		else
			m_PlayerOnlineMap.erase(iter);
	}
}