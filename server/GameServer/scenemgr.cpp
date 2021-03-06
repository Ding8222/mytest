﻿#include "lxnet\base\platform_config.h"
#include "SceneMgr.h"
#include "MapConfig.h"
#include "MapInfo.h"
#include "idmgr.h"
#include "serverlog.h"
#include "objectpool.h"
#include "GlobalDefine.h"
#include "CSVLoad.h"

extern int64 g_currenttime;

//地图场景最大数量
#define SCENE_COUNT_MAX 200

static objectpool<CScene> &ScenePool()
{
	static objectpool<CScene> m(SCENE_COUNT_MAX, "CScene pools");
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
	const std::list<CMapInfo*> maplist = MapConfig.GetMapList();
	for (auto &iter : maplist)
	{
		CMapInfo *info = iter;
		// 只加载普通地图
		if (info->GetMapType() == MapEnum::MapType::EMT_NORMAL)
		{
			if (!AddScene(iter))
			{
				RunStateError("加载普通场景失败，地图ID： %d", (*iter).GetMapID());
				return false;
			}
		}
	}

	RunStateLog("加载所有普通场景成功!总数： %d", maplist.size());

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

void CSceneMgr::GetCurrentInfo(char *buf, size_t buflen)
{
	snprintf(buf, buflen - 1, "普通地图数量:%d\n", (int32)m_SceneMap.size());
	short size;
	for (auto &i : m_SceneMap)
	{
		size = static_cast<short>(strlen(buf));
		i.second->GetCurrentInfo(&buf[size], sizeof(buf) - size - 1);
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

bool CSceneMgr::AddMonster(int32 monsterid, int32 mapid, float x, float y, float z, bool relive, int32 relivecd)
{
	CScene *map = FindScene(mapid);
	if (!map)
	{
		RunStateError("添加Monster：%d 失败！不存在的地图ID：%d", monsterid, mapid);
		return false;
	}

	return map->AddMonster(monsterid, x, y, z, relive, relivecd);
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
	for (auto &i : CSVData::CMapMonsterDB::m_Data)
	{
		std::vector<CSVData::stMapMonster *> *monsterset = i.second;
		// 判断一下所在地图是否加载
		if (FindScene(static_cast<int32>(i.first)))
		{
			std::vector<CSVData::stMapMonster *>::iterator iter = monsterset->begin();
			for (; iter != monsterset->end(); ++iter)
			{
				CSVData::stMapMonster *monster = *iter;
				if (!AddMonster(monster->nMonsterID, monster->nMapID, monster->nX, monster->nY, monster->nZ, monster->bCanRelive, monster->nReliveCD))
				{
					RunStateError("地图：%d 添加monster：%d 失败！");
					return false;
				}
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

