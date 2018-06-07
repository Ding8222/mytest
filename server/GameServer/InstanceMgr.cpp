#include "idmgr.c"
#include "Instance.h"
#include "InstanceMgr.h"
#include "MapInfo.h"
#include "serverlog.h"
#include "objectpool.h"
#include "GlobalDefine.h"

extern int64 g_currenttime;
#define INSTANCE_ID_MAX 2000			//副本数量
#define INSTANCE_ID_DELAY_TIME 300000	//释放延时时间

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
		RunStateError("创建 CInstance 失败!");
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
	Destroy();
}

bool CInstanceMgr::Init()
{
	m_IDPool = idmgr_create(INSTANCE_ID_MAX + 1, INSTANCE_ID_DELAY_TIME);
	if (!m_IDPool)
	{
		RunStateError("创建IDMgr失败!");
		return false;
	}

	m_InstanceSet.resize(INSTANCE_ID_MAX + 1, nullptr);

	return true;
}

void CInstanceMgr::Run()
{
	idmgr_run(m_IDPool);

	ProcessAllInstance();
	CheckAndRemove();
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

void CInstanceMgr::GetCurrentInfo(char *buf, size_t buflen)
{
	snprintf(buf, buflen - 1, "副本地图数量:%d\n待移除副本数量:%d\n", (int32)m_InstanceList.size(), (int32)m_WaitRemove.size());
	short size;
	for (auto &i : m_InstanceList)
	{
		size = static_cast<short>(strlen(buf));
		i->GetCurrentInfo(&buf[size], sizeof(buf) - size - 1);
	}
}

int32 CInstanceMgr::AddInstance(int32 instancebaseid)
{
	int32 id = idmgr_allocid(m_IDPool);
	if (id <= 0)
	{
		RunStateError("为新Instance分配ID失败!, id:%d", id);
		return 0;
	}

	CInstance * newinstance = instance_create();
	if (!newinstance)
	{
		RunStateError("创建CInstance失败!");
		return 0;
	}

	if (newinstance->Init(instancebaseid))
	{
		m_InstanceSet[id] = newinstance;
		m_InstanceList.push_back(newinstance);
		newinstance->SetInsranceID(id);
		return id;
	}

	instance_release(newinstance);

	if (!idmgr_freeid(m_IDPool, id))
		RunStateError("释放ID失败!, ID:%d", id);

	return 0;
}

CInstance *CInstanceMgr::FindInstance(int32 instanceid)
{
	if (instanceid <= 0 || instanceid >= static_cast<int32>(m_InstanceSet.size()))
		return nullptr;

	CInstance * instance = m_InstanceSet[instanceid];
	if (!instance || instance->IsNeedRemove())
		return nullptr;

	return instance;
}

bool CInstanceMgr::AddNPC(int32 npcid, int32 instanceid, float x, float y, float z)
{
	CInstance *instance = FindInstance(instanceid);
	if (!instance)
	{
		RunStateError("添加NPC：%d 失败！不存在的实例副本ID：%d", npcid, instanceid);
		return false;
	}

	return instance->AddNPC(npcid, x, y, z);
}

bool CInstanceMgr::AddMonster(int32 monsterid, int32 instanceid, float x, float y, float z)
{
	CInstance *instance = FindInstance(instanceid);
	if (!instance)
	{
		RunStateError("添加Monster：%d 失败！不存在的实例副本ID：%d", monsterid, instanceid);
		return false;
	}

	return instance->AddMonster(monsterid, x, y, z);
}

bool CInstanceMgr::EnterInstance(CBaseObj * obj, int32 instanceid)
{
	CInstance *instance = FindInstance(instanceid);
	if (instance && !instance->IsNeedRemove())
	{
		return instance->AddObj(obj);
	}

	return false;
}

void CInstanceMgr::ProcessAllInstance()
{
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
	int32 id = instance->GetInsranceID();
	if (id <= 0 || id >= (int32)m_InstanceSet.size())
	{
		RunStateError("要释放的Instance的ID错误!");
		return;
	}
	m_InstanceSet[id] = NULL;

	if (!idmgr_freeid(m_IDPool, id))
	{
		RunStateError("释放ID错误, ID:%d", id);
	}

	instance->SetInsranceID(0);
	instance_release(instance);
}