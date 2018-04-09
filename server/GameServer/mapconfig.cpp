#include"MapConfig.h"
#include"MapInfo.h"
#include"tinyxml2.h"
#include"log.h"
#include "objectpool.h"
#include "serverlog.h"
#include "Config.h"
#include "CSVLoad.h"

using namespace tinyxml2;
#define MAP_ID_MAX 10000

static objectpool<CMapInfo> &MapPool()
{
	static objectpool<CMapInfo> m(MAP_ID_MAX, "CMapInfo pools");
	return m;
}

static CMapInfo *map_create()
{
	CMapInfo *self = MapPool().GetObject();
	if (!self)
	{
		RunStateError("创建 CScene 失败!");
		return NULL;
	}
	new(self) CMapInfo();
	return self;
}

static void map_release(CMapInfo *self)
{
	if (!self)
		return;
	self->~CMapInfo();
	MapPool().FreeObject(self);
}

CMapConfig::CMapConfig()
{
	m_MapList.clear();
	m_MapSet.clear();
	m_TotalMapList.clear();
}

CMapConfig::~CMapConfig()
{
	Destroy();
}

bool CMapConfig::Init()
{
	m_MapSet.resize(MAP_ID_MAX, nullptr);

	for (auto &i : CSVData::CMapDB::m_Data)
	{
		CSVData::stMap *mapinfo = i.second;
		if (mapinfo->nMapID <= 0 || mapinfo->nMapID >= static_cast<int>(m_MapSet.size()))
		{
			RunStateError("地图ID错误！");
			return false;
		}

		m_TotalMapList.insert(mapinfo->nMapID);
		
		if (mapinfo->nLineID != 0 && mapinfo->nLineID != CConfig::Instance().GetLineID())
		{
			continue;
		}
		
		if (m_MapSet[mapinfo->nMapID] != nullptr)
		{
			RunStateError("添加地图失败!地图ID已存在：%d", mapinfo->nMapID);
			return false;
		}

		CMapInfo* newmap = map_create();
		if (!newmap)
		{
			RunStateError("创建CMapInfo失败!");
			return false;
		}

		if (!newmap->Init(mapinfo->nMapID, mapinfo->nType, mapinfo->sMapBar.c_str()))
		{
			RunStateError("初始化加载地图阻挡失败!地图ID：%d", mapinfo->nMapID);
			delete newmap;
			return false;
		}

		newmap->SetMapBirthPoint(mapinfo->nX, mapinfo->nY, mapinfo->nZ);

		m_MapSet[mapinfo->nMapID] = newmap;
		m_MapList.push_back(newmap);
	}
		
	return true;
}

void CMapConfig::Destroy()
{
	std::list<CMapInfo *>::iterator iter = m_MapList.begin();
	for (; iter != m_MapList.end(); ++iter)
	{
		map_release(*iter);
	}
	m_MapList.clear();
	m_TotalMapList.clear();
}

CMapInfo *CMapConfig::FindMapInfo(int32 mapid)
{
	if (mapid <= 0 || mapid > static_cast<int>(m_MapSet.size()))
		return nullptr;

	return m_MapSet[mapid];
}

const std::list<CMapInfo*>& CMapConfig::GetMapList()
{
	return m_MapList;
}

bool CMapConfig::isValidMapID(int32 id)
{
	auto iter = m_TotalMapList.find(id);
	if (iter != m_TotalMapList.end())
		return true;

	return false;
}