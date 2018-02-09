#include "stdfx.h"
#include "SceneMgr.h"
#include "MapConfig.h"
#include "MapInfo.h"
#include "scene.h"
#include "idmgr.c"

extern int64 g_currenttime;

static objectpool<CScene> &ScenePool()
{
	static objectpool<CScene> m(2000, "CScene pools");
	return m;
}

static CScene *scene_create()
{
	CScene *self = ScenePool().GetObject();
	if (!self)
	{
		log_error("创建 CScene 失败!");
		return NULL;
	}
	new(self) CScene();
	return self;
}

static void scene_release(CScene *self)
{
	if (!self)
		return;
	self->~CScene();
	ScenePool().FreeObject(self);
}

CScenemgr::CScenemgr()
{
	m_SceneMap.clear();
	m_InstanceList.clear();
	m_WaitRemove.clear();
	m_InstanceSet.clear();
	m_IDPool = nullptr;
}

CScenemgr::~CScenemgr()
{
	Destroy();
}

bool CScenemgr::Init()
{
	// 加载普通地图
	const std::list<CMapInfo*> maplist = CMapConfig::Instance().GetMapList();
	for (auto &iter : maplist)
	{
		if (!AddScene(iter))
		{
			log_error("加载普通场景失败，地图ID： %d", (*iter).GetMapID());
			return false;
		}
	}

	m_IDPool = idmgr_create(INSTANCE_ID_MAX + 1, INSTANCE_ID_DELAY_TIME);
	if (!m_IDPool)
	{
		log_error("创建IDMgr失败!");
		return false;
	}

	m_InstanceSet.resize(INSTANCE_ID_MAX + 1, nullptr);
	
	return true;
}

void CScenemgr::Destroy()
{
	std::unordered_map<int, CScene *>::iterator iter = m_SceneMap.begin();
	for (; iter != m_SceneMap.end(); ++iter)
	{
		iter->second->Destroy();
	}
	m_SceneMap.clear();

	for (std::list<CScene*>::iterator itr = m_InstanceList.begin(); itr != m_InstanceList.end(); ++itr)
	{
		ReleaseInstanceAndID(iter->second);
	}
	m_InstanceList.clear();

	std::list<CScene *>::iterator iterw = m_WaitRemove.begin();
	for (; iterw != m_WaitRemove.end(); ++iterw)
	{
		ReleaseInstanceAndID(*iterw);
	}
	m_WaitRemove.clear();

	m_InstanceSet.clear();

	if (m_IDPool)
	{
		idmgr_release(m_IDPool);
		m_IDPool = nullptr;
	}
}

void CScenemgr::Run()
{
	// 普通地图
	for (auto &i : m_SceneMap)
	{
		i.second->Run();
	}

	// 副本
	std::list<CScene*>::iterator iter, tempiter;
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

void CScenemgr::CheckAndRemove()
{
	CScene *scene;
	while (!m_WaitRemove.empty())
	{
		scene = m_WaitRemove.front();
		if (!scene->CanRemove(g_currenttime))
			break;
		ReleaseInstanceAndID(scene);
		m_WaitRemove.pop_front();
	}
}

bool CScenemgr::AddScene(CMapInfo* mapconfig)
{
	if (!mapconfig)
		return false;

	CScene * newscene = scene_create();
	if (!newscene)
	{
		log_error("创建CScene失败!");
		return false;
	}

	if (newscene->Init(mapconfig))
	{
		m_SceneMap[newscene->GetMapID()] = newscene;
		return true;
	}
	else
		scene_release(newscene);
	
	return false;
}

int CScenemgr::AddInstance(int instancebaseid)
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

	CScene * newscene = scene_create();
	if (!newscene)
	{
		log_error("创建CScene失败!");
		return 0;
	}

	if (newscene->Init(mapconfig))
	{
		m_InstanceSet[id] = newscene;
		m_InstanceList.push_back(newscene);
		newscene->SetInsranceID(id);
		return true;
	}
	
	scene_release(newscene);

	if (!idmgr_freeid(m_IDPool, id))
		log_error("释放ID失败!, ID:%d", id);

	return 0;
}

CScene *CScenemgr::FindScene(int mapid)
{
	auto iter = m_SceneMap.find(mapid);
	if (iter != m_SceneMap.end())
		return iter->second;

	return nullptr;
}

void CScenemgr::ReleaseInstanceAndID(CScene *scene)
{
	if (!scene)
		return;
	int id = scene->GetInsranceID();
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

	scene->SetInsranceID(0);
	scene_release(scene);
}
