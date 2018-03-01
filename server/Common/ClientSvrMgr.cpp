
#include <vector>
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
}

CClientSvrMgr::~CClientSvrMgr()
{
	Destroy();
}

bool CClientSvrMgr::Init()
{
	m_ClientSvrSet.resize(CLIENT_ID_MAX + 1, nullptr);
	return true;
}

void CClientSvrMgr::Destroy()
{
	for (auto &i : m_ClientSvrSet)
	{
		clientsvr_release(i);
		i = nullptr;
	}
	m_ClientSvrSet.clear();
}

void CClientSvrMgr::AddClientSvr(int32 clientid, int serverid, int servertype)
{
	if (clientid <= 0 || clientid >= static_cast<int>(m_ClientSvrSet.size()))
		return;

	ClientSvr *cl = m_ClientSvrSet[clientid];
	if (cl)
	{
		switch (servertype)
		{
		case ServerEnum::EST_GAME:
		{
			cl->nGameServerID = serverid;
			break;
		}
		case ServerEnum::EST_GATE:
		{
			cl->nGateID = serverid;
			break;
		}
		}
	}
	else
	{
		ClientSvr *newclientsvr = clientsvr_create();
		if (!newclientsvr)
		{
			log_error("创建ClientSvr失败!");
			return;
		}

		switch (servertype)
		{
		case ServerEnum::EST_GAME:
		{
			newclientsvr->nGameServerID = serverid;
			break;
		}
		case ServerEnum::EST_GATE:
		{
			newclientsvr->nGateID = serverid;
			break;
		}
		}
		assert(nullptr == m_ClientSvrSet[clientid]);
		m_ClientSvrSet[clientid] = newclientsvr;
	}
}

void CClientSvrMgr::DelClientSvr(int32 clientid)
{
	if (clientid <= 0 || clientid >= static_cast<int>(m_ClientSvrSet.size()))
	{
		log_error("要释放的ClientSvr的ID错误!");
		return;
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
