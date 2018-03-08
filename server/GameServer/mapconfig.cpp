#include"MapConfig.h"
#include"MapInfo.h"
#include"tinyxml2.h"
#include"log.h"
#include "objectpool.h"
#include "serverlog.h"
#include "Config.h"

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

	const char *filename = "./data/maplist.xml";
	XMLDocument doc;
	if (doc.LoadFile(filename) != XML_SUCCESS)
	{
		RunStateError("加载 %s 失败!", filename);
		return false;
	}

	// 将maplist.xml中的信息添加到m_MapList

	XMLElement *pinfo = doc.FirstChildElement("maplist");
	if (!pinfo)
	{
		RunStateError("没有找到字段： 'maplist'");
		return false;
	}

	pinfo = pinfo->FirstChildElement("maps");
	if (!pinfo)
	{
		RunStateError("没有找到字段： 'maps'");
		return false;
	}

	while (pinfo)
	{
		int32 mapid = 0;
		if (pinfo->QueryIntAttribute("mapid", &mapid) != XML_SUCCESS)
		{
			RunStateError("没有找到字段： 'mapid'");
			return false;
		}

		if (mapid <= 0 || mapid >= static_cast<int>(m_MapSet.size()))
		{
			RunStateError("地图ID错误！");
			return false;
		}

		m_TotalMapList.insert(mapid);

		int32 lineid = 0;
		if (pinfo->QueryIntAttribute("lineid", &lineid) != XML_SUCCESS)
		{
			RunStateError("没有找到字段： 'lineid'");
			return false;
		}

		if (lineid < 0)
		{
			RunStateError("线路ID错误！");
			return false;
		}

		if (lineid != 0 && lineid != CConfig::Instance().GetLineID())
		{
			pinfo = pinfo->NextSiblingElement("maps");
			continue;
		}

		std::string filename = pinfo->Attribute("bar_filename");
		if (filename.empty())
		{
			RunStateError("没有找到字段： 'bar_filename'");
			return false;
		}

		if (m_MapSet[mapid] != nullptr)
		{
			RunStateError("添加地图失败!地图ID已存在：%d", mapid);
			return false;
		}

		CMapInfo* newmap = map_create();
		if (!newmap)
		{
			RunStateError("创建CMapInfo失败!");
			return false;
		}

		if (!newmap->Init(mapid, filename.c_str()))
		{
			RunStateError("初始化加载地图配置失败!地图ID：%d", mapid);
			return false;
		}

		m_MapSet[mapid] = newmap;
		m_MapList.push_back(newmap);
		
		pinfo = pinfo->NextSiblingElement("maps");
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