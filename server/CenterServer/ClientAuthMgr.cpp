#include "ClientAuthMgr.h"
#include "CentServerMgr.h"
#include "ServerStatusMgr.h"
#include "objectpool.h"
#include "ServerLog.h"

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

void CClientAuthMgr::Destroy(bool bLoginDisconnect)
{
	for (auto &i : m_ClientInfoSet)
	{
		clientauthinfo_release(i);
		i = nullptr;
	}
	if(!bLoginDisconnect)
		m_ClientInfoSet.clear();
}

// 添加认证信息
void CClientAuthMgr::AddClientAuthInfo(Msg *pMsg, int32 clientid, int32 serverid)
{
	if (clientid <= 0 || clientid >= static_cast<int>(m_ClientInfoSet.size()))
	{
		RunStateError("要添加的ClientAuthInfo的ID错误!");
		return;
	}

	assert(m_ClientInfoSet[clientid] == nullptr);

	ClientAuthInfo *newinfo = clientauthinfo_create();
	if (!newinfo)
	{
		log_error("创建ClientAuthInfo失败!");
		return;
	}

	netData::Auth msg;
	_CHECK_PARSE_(pMsg, msg);
	
	newinfo->nLoginSvrID = serverid;
	newinfo->Token = msg.setoken();
	newinfo->Secret = msg.ssecret();

	m_ClientInfoSet[clientid] = newinfo;

	CCentServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_DB, clientid);
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

// 发送认证信息到逻辑服
void CClientAuthMgr::SendAuthInfoToLogic(Msg *pMsg, int32 clientid)
{

}

void CClientAuthMgr::SendLoadPlayerDataToLogic(Msg *pMsg, int32 clientid)
{
	CCentServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_GAME, clientid);
}