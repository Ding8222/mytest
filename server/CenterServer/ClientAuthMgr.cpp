#include "idmgr.c"
#include "ClientAuthMgr.h"
#include "CentServerMgr.h"
#include "ServerStatusMgr.h"
#include "objectpool.h"
#include "ServerLog.h"
#include "Config.h"

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
	m_IDPool = nullptr;
}

CClientAuthMgr::~CClientAuthMgr()
{
	Destroy();
}

bool CClientAuthMgr::Init()
{
	m_IDPool = idmgr_create(CLIENT_ID_MAX + 1, CLIENT_ID_MAX);
	if (!m_IDPool)
	{
		RunStateError("创建IDMgr失败!");
		return false;
	}

	m_ClientInfoSet.resize(CLIENT_ID_MAX + 1, nullptr);

	return true;
}

void CClientAuthMgr::Run()
{
	idmgr_run(m_IDPool);
}

void CClientAuthMgr::Destroy(bool bLoginDisconnect)
{
	for (size_t i = 0; i < m_ClientInfoSet.size(); ++i)
	{
		DelClientAuthInfo(i);
	}

	if(!bLoginDisconnect)
		m_ClientInfoSet.clear();

	if (!bLoginDisconnect && m_IDPool)
	{
		idmgr_release(m_IDPool);
		m_IDPool = nullptr;
	}
}

// 添加认证信息
void CClientAuthMgr::AddClientAuthInfo(Msg *pMsg, int32 clientid, int32 serverid)
{
	int id = idmgr_allocid(m_IDPool);
	if (id <= 0)
	{
		RunStateError("为新Instance分配ID失败!, id:%d", id);
		return ;
	}

	assert(m_ClientInfoSet[id] == nullptr);

	ClientAuthInfo *newinfo = clientauthinfo_create();
	if (!newinfo)
	{
		if (!idmgr_freeid(m_IDPool, id))
		{
			log_error("释放ID错误, ID:%d", id);
		}
		log_error("创建ClientAuthInfo失败!");
		return;
	}

	netData::Auth msg;
	_CHECK_PARSE_(pMsg, msg);
	
	newinfo->nClientID = clientid;
	newinfo->nLoginSvrID = serverid;
	newinfo->Token = msg.setoken();
	newinfo->Secret = msg.ssecret();

	m_ClientInfoSet[id] = newinfo;

	CCentServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_DB, id, CConfig::Instance().GetDBID());
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

	if (!idmgr_freeid(m_IDPool, clientid))
	{
		log_error("释放ID错误, ID:%d", clientid);
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
