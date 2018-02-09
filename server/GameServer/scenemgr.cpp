﻿#include "stdfx.h"
#include "SceneMgr.h"
#include "MapConfig.h"
#include "MapInfo.h"
#include "Scene.h"
#include "idmgr.c"

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
}

void CScenemgr::Run()
{
	// 普通地图
	for (auto &i : m_SceneMap)
	{
		i.second->Run();
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

CScene *CScenemgr::FindScene(int mapid)
{
	auto iter = m_SceneMap.find(mapid);
	if (iter != m_SceneMap.end())
		return iter->second;

	return nullptr;
}

