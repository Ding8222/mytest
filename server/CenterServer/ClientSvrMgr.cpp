#include <vector>
#include "idmgr.c"
#include "msgbase.h"
#include "ClientSvrMgr.h"
#include "objectpool.h"
#include "GlobalDefine.h"
#include "log.h"

static objectpool<ClientSvr> &ClientSvrPool()
{
	static objectpool<ClientSvr> m(CLIENT_ID_MAX, "ClientSvr pools");
	return m;
}

static ClientSvr *clientsvr_create()
{
	ClientSvr *self = ClientSvrPool().GetObject();
	if (!self)
	{
		log_error("创建 ClientSvr 失败!");
		return NULL;
	}
	new(self) ClientSvr();
	return self;
}

static void clientsvr_release(ClientSvr *self)
{
	if (!self)
		return;
	self->~ClientSvr();
	ClientSvrPool().FreeObject(self);
}

CClientSvrMgr::CClientSvrMgr()
{
	m_ClientSvrSet.clear();
	m_IDPool = nullptr;
}

CClientSvrMgr::~CClientSvrMgr()
{
	Destroy();
}

bool CClientSvrMgr::Init()
{
	m_IDPool = idmgr_create(CLIENT_ID_MAX + 1, CLIENT_ID_MAX);
	if (!m_IDPool)
	{
		log_error("创建IDMgr失败!");
		return false;
	}

	m_ClientSvrSet.resize(CLIENT_ID_MAX + 1, nullptr);
	return true;
}

void CClientSvrMgr::Run()
{
	idmgr_run(m_IDPool);
}

void CClientSvrMgr::Destroy()
{
	for (size_t i = 0; i < m_ClientSvrSet.size(); ++i)
	{
		DelClientSvr(i);
	}
	m_ClientSvrSet.clear();

	if (m_IDPool)
	{
		idmgr_release(m_IDPool);
		m_IDPool = nullptr;
	}
}

int32 CClientSvrMgr::AddClientSvr(int32 clientid, int32 serverid, int32 gateid)
{
	int id = idmgr_allocid(m_IDPool);
	if (id <= 0)
	{
		log_error("为新ClientSvr分配ID失败!, id:%d", id);
		return 0;
	}
	
	ClientSvr *cl = m_ClientSvrSet[id];
	assert(!cl);
	if (!cl)
	{
		ClientSvr *newclientsvr = clientsvr_create();
		if (!newclientsvr)
		{
			if (!idmgr_freeid(m_IDPool, id))
			{
				log_error("释放ID错误, ID:%d", id);
			}
			log_error("创建ClientSvr失败!");
			return 0;
		}

		newclientsvr->nClientID = clientid;
		newclientsvr->nGameServerID = serverid;
		newclientsvr->nGateID = gateid;

		m_ClientSvrSet[id] = newclientsvr;
	}
	else
	{
		if (!idmgr_freeid(m_IDPool, id))
		{
			log_error("释放ID错误, ID:%d", id);
		}
		return 0;
	}
	return id;
}

void CClientSvrMgr::UpdateClientGameSvr(int32 clientid, int32 serverid)
{
	if (clientid <= 0 || clientid >= static_cast<int>(m_ClientSvrSet.size()))
	{
		log_error("要释放的ClientSvr的ID错误!");
		return;
	}

	assert(m_ClientSvrSet[clientid]);
	m_ClientSvrSet[clientid]->nGameServerID = serverid;
}

void CClientSvrMgr::DelClientSvr(int32 clientid)
{
	if (clientid <= 0 || clientid >= static_cast<int>(m_ClientSvrSet.size()))
	{
		log_error("要释放的ClientSvr的ID错误!");
		return;
	}

	if (!idmgr_freeid(m_IDPool, clientid))
	{
		log_error("释放ID错误, ID:%d", clientid);
	}

	clientsvr_release(m_ClientSvrSet[clientid]);
	m_ClientSvrSet[clientid] = nullptr;
}

ClientSvr *CClientSvrMgr::GetClientSvr(int32 id)
{
	if (id <= 0 || id >= static_cast<int>(m_ClientSvrSet.size()))
		return nullptr;

	assert(m_ClientSvrSet[id]);
	return m_ClientSvrSet[id];
}
