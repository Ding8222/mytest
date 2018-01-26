#include"mapconfig.h"
#include"mapinfo.h"
#include"scenemgr.h"
#include"tinyxml2.h"
#include"log.h"

using namespace tinyxml2;

CMapConfig::CMapConfig()
{
	m_MapList.clear();
}

CMapConfig::~CMapConfig()
{
	auto iterB = m_MapList.begin();
	auto iterE = m_MapList.end();
	for (; iterB != iterE; iterB++)
	{
		delete iterB->second;
	}

	m_MapList.clear();
}

bool CMapConfig::Init()
{
	const char *filename = "./data/maplist.xml";
	XMLDocument doc;
	if (doc.LoadFile(filename) != XML_SUCCESS)
	{
		log_error("load %s failed!", filename);
		return false;
	}

	// 将maplist.xml中的信息添加到m_MapList

	XMLElement *pinfo = doc.FirstChildElement("maplist");
	if (!pinfo)
	{
		log_error("not find first child element, element name: 'maplist'");
		return false;
	}

	pinfo = pinfo->FirstChildElement("maps");
	if (!pinfo)
	{
		log_error("not find first child element, element name: 'maps'");
		return false;
	}

	while (pinfo)
	{
		CMapInfo* m_mapinfo = new CMapInfo;
		if (!m_mapinfo)
		{
			log_error("application memory failed! new m_mapinfo ");
			return false;
		}

		int mapid = 0;

		if (pinfo->QueryIntAttribute("mapid", &mapid) != XML_SUCCESS)
		{
			log_error("query int attribute failed, attribute name: 'mapid'");
			return false;
		}

		if (mapid <= 0)
		{
			log_error("error: mapid <= 0 !");
			return false;
		}

		std::string filename = pinfo->Attribute("bar_filename");
		if (filename.empty())
		{
			log_error("attribute failed, attribute name: 'bar_filename'");
			return false;
		}
		m_mapinfo->Init(mapid, filename);

		auto iter = m_MapList.find(mapid);
		if (iter != m_MapList.end())
		{
			log_error("add map list error ,mapid: %d already exist!", mapid);
			return false;
		}

		if (!LoadBar(m_mapinfo))
		{
			log_error("load map bar error ,mapid: %d", mapid);
			return false;
		}

		m_MapList.insert(std::make_pair(mapid, m_mapinfo));
		pinfo = pinfo->NextSiblingElement("maps");
	}
	
	return true;
}

void CMapConfig::Destroy()
{

}

bool CMapConfig::LoadBar(CMapInfo* map)
{
	if (!map)
		return false;

	XMLDocument doc;
	if (doc.LoadFile(map->GetBarFileName().c_str()) != XML_SUCCESS)
	{
		log_error("load %s failed!", map->GetBarFileName().c_str());
		return false;
	}

	// 添加地图阻挡点信息

	XMLElement *pinfo = doc.FirstChildElement("bar_map");
	if (!pinfo)
	{
		log_error("not find first child element, element name: 'bar_map'");
		return false;
	}

	int width = 0;
	int height = 0;
	if (pinfo->QueryIntAttribute("width", &width) != XML_SUCCESS)
	{
		log_error("query int attribute failed, attribute name: 'width'");
		return false;
	}

	if (width <= 0)
	{
		log_error("map width <= 0 ,mapid:%d ", map->GetMapID());
		return false;
	}
	
	if (pinfo->QueryIntAttribute("height", &height) != XML_SUCCESS)
	{
		log_error("query int attribute failed, attribute name: 'height'");
		return false;
	}

	if (height <= 0)
	{
		log_error("map height <= 0 ,mapid:%d ", map->GetMapID());
		return false;
	}
	
	bool* barinfo = new bool[width * height];

	if (!barinfo)
	{
		log_error("application memory failed! new m_barinfo ");
		return false;
	}
	memset(barinfo, 0, width * height * sizeof(bool));
	
	pinfo = pinfo->FirstChildElement("bar");

	while (pinfo)
	{
		int row = 0;
		int col = 0;

		if (pinfo->QueryIntAttribute("row", &row) != XML_SUCCESS)
		{
			log_error("query int attribute failed, attribute name: 'row'");
			delete(barinfo);
			return false;
		}

		if (row < 0 || row > width)
		{
			log_error("map bar row < 0 or row > m_width ,mapid:%d ", map->GetMapID());
			delete(barinfo);
			return false;
		}

		if (pinfo->QueryIntAttribute("col", &col) != XML_SUCCESS)
		{
			log_error("query int attribute failed, attribute name: 'col'");
			delete(barinfo);
			return false;
		}

		if (col < 0 || col > height)
		{
			log_error("map bar col < 0 or col > m_height ,mapid:%d ", map->GetMapID());
			delete(barinfo);
			return false;
		}

		barinfo[row * col] = true;

		pinfo = pinfo->NextSiblingElement("bar");
	}

	map->SetMapBarInfo(width, height, barinfo);
	return true;
}

const CMapInfo *CMapConfig::GetMapInfo(int mapid)
{
	auto iter = m_MapList.find(mapid);
	if (iter != m_MapList.end())
	{
		return iter->second;
	}

	return nullptr;
}

const std::unordered_map<int, CMapInfo*>& CMapConfig::GetMapList()
{
	return m_MapList;
}