#include "stdfx.h"
#include "idmgr.c"
#include "Instance.h"
#include "InstanceMgr.h"
#include "MapConfig.h"

extern int64 g_currenttime;
#define INSTANCE_ID_MAX 2000

static objectpool<CInstance> &InstancePool()
{
	static objectpool<CInstance> m(INSTANCE_ID_MAX, "CInstance pools");
	return m;
}

static CInstance *instance_create()
{
	CInstance *self = InstancePool().GetObject();
	if (!self)
	{
		log_error("创建 CInstance 失败!");
		return NULL;
	}
	new(self) CInstance();
	return self;
}

static void instance_release(CInstance *self)
{
	if (!self)
		return;
	self->~CInstance();
	InstancePool().FreeObject(self);
}

CInstanceMgr::CInstanceMgr()
{
	m_InstanceList.clear();
	m_WaitRemove.clear();
	m_InstanceSet.clear();
	m_IDPool = nullptr;
}

CInstanceMgr::~CInstanceMgr()
{

}

bool CInstanceMgr::Init()
{
	m_IDPool = idmgr_create(INSTANCE_ID_MAX + 1, INSTANCE_ID_DELAY_TIME);
	if (!m_IDPool)
	{
		log_error("创建IDMgr失败!");
		return false;
	}

	m_InstanceSet.resize(INSTANCE_ID_MAX + 1, nullptr);

	return true;
}

void CInstanceMgr::Run()
{
	// 副本
	std::list<CInstance*>::iterator iter, tempiter;
	for (iter = m_InstanceList.begin(); iter != m_InstanceList.end();)
	{
		tempiter = iter;
		++iter;

		if ((*tempiter)->IsNeedRemove())
		{
			m_WaitRemove.push_back(*tempiter);
			m_InstanceList.erase(tempiter);
			continue;
		}

		(*tempiter)->Run();
	}
}

void CInstanceMgr::Destroy()
{
	std::list<CInstance *>::iterator iter = m_InstanceList.begin();
	for (; iter != m_InstanceList.end(); ++iter)
	{
		ReleaseInstanceAndID(*iter);
	}
	m_InstanceList.clear();

	iter = m_WaitRemove.begin();
	for (; iter != m_WaitRemove.end(); ++iter)
	{
		ReleaseInstanceAndID(*iter);
	}
	m_WaitRemove.clear();

	m_InstanceSet.clear();

	if (m_IDPool)
	{
		idmgr_release(m_IDPool);
		m_IDPool = nullptr;
	}
}

int CInstanceMgr::AddInstance(int instancebaseid)
{
	CMapInfo* mapconfig = CMapConfig::Instance().FindMapInfo(instancebaseid);

	if (!mapconfig)
		return 0;

	int id = idmgr_allocid(m_IDPool);
	if (id <= 0)
	{
		log_error("为新Instance分配ID失败!, id:%d", id);
		return 0;
	}

	CInstance * newinstance = instance_create();
	if (!newinstance)
	{
		log_error("创建CInstance失败!");
		return 0;
	}

	if (newinstance->Init(mapconfig))
	{
		m_InstanceSet[id] = newinstance;
		m_InstanceList.push_back(newinstance);
		newinstance->SetInsranceID(id);
		return true;
	}

	instance_release(newinstance);

	if (!idmgr_freeid(m_IDPool, id))
		log_error("释放ID失败!, ID:%d", id);

	return 0;
}

void CInstanceMgr::CheckAndRemove()
{
	CInstance *instance;
	while (!m_WaitRemove.empty())
	{
		instance = m_WaitRemove.front();
		if (!instance->CanRemove(g_currenttime))
			break;
		ReleaseInstanceAndID(instance);
		m_WaitRemove.pop_front();
	}
}

void CInstanceMgr::ReleaseInstanceAndID(CInstance *instance)
{
	if (!instance)
		return;
	int id = instance->GetInsranceID();
	if (id <= 0 || id >= (int)m_InstanceSet.size())
	{
		log_error("要释放的Instance的ID错误!");
		return;
	}
	m_InstanceSet[id] = NULL;

	if (!idmgr_freeid(m_IDPool, id))
	{
		log_error("释放ID错误, ID:%d", id);
	}

	instance->SetInsranceID(0);
	instance_release(instance);
}