#include "idmgr.c"
#include "ClientAuthMgr.h"
#include "CentServerMgr.h"
#include "ServerStatusMgr.h"
#include "objectpool.h"
#include "ServerLog.h"
#include "Config.h"

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
	m_bDBSvrReady = false;
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

void CClientAuthMgr::Destroy()
{
	for (size_t i = 0; i < m_ClientInfoSet.size(); ++i)
	{
		clientauthinfo_release(m_ClientInfoSet[i]);
		m_ClientInfoSet[i] = nullptr;
	}

	m_ClientInfoSet.clear();
}

void CClientAuthMgr::AsLoginServerDisconnect()
{
	for (size_t i = 0; i < m_ClientInfoSet.size(); ++i)
	{
		clientauthinfo_release(m_ClientInfoSet[i]);
		m_ClientInfoSet[i] = nullptr;
	}

	m_PlayerLoginMap.clear();
}

// 添加认证信息
void CClientAuthMgr::QueryAuth(Msg *pMsg, int32 clientid, int32 serverid)
{
	netData::Auth msg;
	_CHECK_PARSE_(pMsg, msg);

	if (!GetIsDBSvrReady())
	{
		RunStateError("DB服务器没有启动！");
		netData::AuthRet SendMsg;
		SendMsg.set_ncode(netData::AuthRet::EC_DBSTAUTS);
		CCentServerMgr::Instance().SendMsgToServer(SendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET, ServerEnum::EST_LOGIN, clientid, serverid);
		return;
	}

	bool bWaitKick = false;
	auto iter = m_PlayerOnlineMap.find(msg.account());
	if (iter != m_PlayerOnlineMap.end())
	{
		if (iter->second > 0)
		{
			auto iterW = m_PlayerLoginMap.find(msg.account());
			if (iterW == m_PlayerLoginMap.end())
			{
				RunStateLog("账号%s在线，尝试连接的clientid：%d，尝试踢下原有玩家", msg.account().c_str(), clientid);
				svrData::KickClient SendMsg;
				CCentServerMgr::Instance().SendMsgToServer(SendMsg, SERVER_TYPE_MAIN, SVR_SUB_KICKCLIENT, ServerEnum::EST_GATE, iter->second);
				bWaitKick = true;
			}
			else
			{
				RunStateLog("正在等待踢出账号%s，操作无效", msg.account().c_str());
				netData::AuthRet SendMsg;
				SendMsg.set_ncode(netData::AuthRet::EC_WATING);
				CCentServerMgr::Instance().SendMsgToServer(SendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET, ServerEnum::EST_LOGIN, clientid, serverid);
				return;
			}
		}
		else
		{
			RunStateLog("账号%s正在认证中，操作无效", msg.account().c_str());
			netData::AuthRet SendMsg;
			SendMsg.set_ncode(netData::AuthRet::EC_AUTHING);
			CCentServerMgr::Instance().SendMsgToServer(SendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET, ServerEnum::EST_LOGIN, clientid, serverid);
			return;
		}
	}

	ClientAuthInfo *newinfo = clientauthinfo_create();
	if (!newinfo)
	{
		RunStateError("创建ClientAuthInfo失败!");
		return;
	}

	newinfo->nLoginSvrID = serverid;
	newinfo->Account = msg.account();
	newinfo->Secret = msg.secret();
	m_ClientInfoSet[clientid] = newinfo;

	if(bWaitKick)
		m_PlayerLoginMap[msg.account()] = clientid;
	else
	{
		m_PlayerOnlineMap[msg.account()] = 0;
		CCentServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_DB, clientid, CConfig::Instance().GetDBID());
	}
}

// 移除认证信息
void CClientAuthMgr::DelClientAuthInfo(int32 clientid)
{
	if (clientid <= 0 || clientid >= static_cast<int>(m_ClientInfoSet.size()))
	{
		log_error("要释放的ClientAuthInfo的ID错误!%d", clientid);
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

void CClientAuthMgr::SetGuid(const std::string &account, int64 guid)
{
	auto iter = m_PlayerOnlineMap.find(account);
	assert(iter != m_PlayerOnlineMap.end());
	if (iter != m_PlayerOnlineMap.end())
	{
		iter->second = guid;
	}
}

void CClientAuthMgr::SetPlayerOnline(const std::string &account, int64 guid)
{
	auto iter = m_PlayerOnlineMap.find(account);
	assert(iter == m_PlayerOnlineMap.end());
	if (iter == m_PlayerOnlineMap.end())
	{
		m_PlayerOnlineMap[account] = guid;
	}
}

void CClientAuthMgr::SetPlayerOffline(const std::string &account)
{
	auto iter = m_PlayerOnlineMap.find(account);
	assert(iter != m_PlayerOnlineMap.end());
	if (iter != m_PlayerOnlineMap.end())
	{
		auto iterF = m_PlayerLoginMap.find(account);
		if (iterF != m_PlayerLoginMap.end())
		{
			assert(iterF->second);
			ClientAuthInfo *info = m_ClientInfoSet[iterF->second];
			m_PlayerOnlineMap[account] = 0;
			netData::Auth SendMsg;
			SendMsg.set_account(info->Account);
			SendMsg.set_secret(info->Secret);
			CCentServerMgr::Instance().SendMsgToServer(SendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH, ServerEnum::EST_DB, iterF->second, CConfig::Instance().GetDBID());
			m_PlayerLoginMap.erase(iterF);
		}
		else
			m_PlayerOnlineMap.erase(iter);
	}
}