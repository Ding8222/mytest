#include "platform_config.h"
#include "SceneMgr.h"
#include "MapConfig.h"
#include "MapInfo.h"
#include "Scene.h"
#include "idmgr.c"
#include "serverlog.h"
#include "objectpool.h"
#include "GlobalDefine.h"
#include "CSVLoad.h"

extern int64 g_currenttime;

static objectpool<CScene> &ScenePool()
{
	static objectpool<CScene> m(SCENE_ID_MAX, "CScene pools");
	return m;
}

static CScene *scene_create()
{
	CScene *self = ScenePool().GetObject();
	if (!self)
	{
		RunStateError("创建 CScene 失败!");
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

CSceneMgr::CSceneMgr()
{
	m_SceneMap.clear();
}

CSceneMgr::~CSceneMgr()
{
	Destroy();
}

bool CSceneMgr::Init()
{
	// 加载普通地图
	const std::list<CMapInfo*> maplist = CMapConfig::Instance().GetMapList();
	for (auto &iter : maplist)
	{
		CMapInfo *info = iter;
		// 只加载普通地图
		if (info->GetMapType() == MapEnum::MapType::EMT_NORMAL)
		{
			if (AddScene(iter))
				RunStateLog("加载普通场景成功，地图ID： %d", (*iter).GetMapID());
			else
			{
				RunStateError("加载普通场景失败，地图ID： %d", (*iter).GetMapID());
				return false;
			}
		}
	}

	if (!LoadNPC())
	{
		RunStateError("加载NPC失败！");
		return false;
	}

	if (!LoadMonster())
	{
		RunStateError("加载Monster失败！");
		return false;
	}

	return true;
}

void CSceneMgr::Destroy()
{
	std::unordered_map<int32, CScene *>::iterator iter = m_SceneMap.begin();
	for (; iter != m_SceneMap.end(); ++iter)
	{
		scene_release(iter->second);
	}
	m_SceneMap.clear();
}

void CSceneMgr::Run()
{
	// 普通地图
	for (auto &i : m_SceneMap)
	{
		i.second->Run();
	}
}

bool CSceneMgr::AddNPC(int32 npcid, int32 mapid, float x, float y, float z)
{
	CScene *map = FindScene(mapid);
	if (!map)
	{
		RunStateError("添加NPC：%d 失败！不存在的地图ID：%d", npcid, mapid);
		return false;
	}
	
	return map->AddNPC(npcid, x, y, z);
}

bool CSceneMgr::AddMonster(int32 monsterid, int32 mapid, float x, float y, float z)
{
	CScene *map = FindScene(mapid);
	if (!map)
	{
		RunStateError("添加Monster：%d 失败！不存在的地图ID：%d", monsterid, mapid);
		return false;
	}

	return map->AddMonster(monsterid, x, y, z);
}

bool CSceneMgr::AddScene(CMapInfo* mapconfig)
{
	if (!mapconfig)
		return false;

	CScene * newscene = scene_create();
	if (!newscene)
	{
		RunStateError("创建CScene失败!");
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

bool CSceneMgr::LoadNPC()
{
	for (auto &i : CSVData::CNPCDB::m_Data)
	{
		CSVData::stNPC *npc = i.second;
		// 判断一下所在地图是否加载
		if (FindScene(npc->nMapID))
		{
			if (!AddNPC(npc->nNPCID, npc->nMapID, npc->nX, npc->nY, npc->nZ))
			{
				RunStateError("地图：%d 添加NPC：%d 失败！");
				return false;
			}
		}
	}
	return true;
}

bool CSceneMgr::LoadMonster()
{
	for (auto &i : CSVData::CMonsterDB::m_Data)
	{
		CSVData::stMonster *npc = i.second;
		// 判断一下所在地图是否加载
		if (FindScene(npc->nMapID))
		{
			if (!AddMonster(npc->nMonsterID, npc->nMapID, npc->nX, npc->nY, npc->nZ))
			{
				RunStateError("地图：%d 添加monster：%d 失败！");
				return false;
			}
		}
	}
	return true;
}

CScene *CSceneMgr::FindScene(int32 mapid)
{
	auto iter = m_SceneMap.find(mapid);
	if (iter != m_SceneMap.end())
		return iter->second;

	return nullptr;
}

