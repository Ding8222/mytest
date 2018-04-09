#include"MapInfo.h"
#include"tinyxml2.h"
#include"log.h"
#include "serverlog.h"

using namespace tinyxml2;

CMapInfo::CMapInfo()
{
	m_MapID = 0;
	m_MapType = 0;
	m_Width = 0;
	m_Height = 0;
	m_BirthPoint_X = 0.0f;
	m_BirthPoint_Y = 0.0f;
	m_BirthPoint_Z = 0.0f;
	m_BarInfo = nullptr;
}

CMapInfo::~CMapInfo()
{
	Destroy();
}

bool CMapInfo::Init(int32 mapid, int32 type, const char *bar_filename)
{
	if (!bar_filename)
	{
		RunStateError("地图配置阻挡点文件路径不存在！");
		return false;
	}

	m_MapID = mapid;
	m_MapType = type;

	XMLDocument doc;
	if (doc.LoadFile(bar_filename) != XML_SUCCESS)
	{
		RunStateError("加载 %s 失败!", bar_filename);
		return false;
	}

	// 添加地图阻挡点信息

	XMLElement *pinfo = doc.FirstChildElement("bar_map");
	if (!pinfo)
	{
		RunStateError("没有找到字段： 'bar_map'");
		return false;
	}

	if (pinfo->QueryIntAttribute("width", &m_Width) != XML_SUCCESS)
	{
		RunStateError("没有找到字段： 'width'");
		return false;
	}

	if (m_Width <= 0)
	{
		RunStateError("地图宽 <= 0 ,地图ID：%d ", mapid);
		return false;
	}

	if (pinfo->QueryIntAttribute("height", &m_Height) != XML_SUCCESS)
	{
		RunStateError("没有找到字段： 'height'");
		return false;
	}

	if (m_Height <= 0)
	{
		RunStateError("地图宽 <= 0 ,地图ID：%d ", mapid);
		return false;
	}

	bool* barinfo = nullptr;

	try {
		barinfo = new bool[m_Width * m_Height];
	}
	catch (std::bad_alloc &) {
		barinfo = nullptr;
	}

	if (!barinfo)
	{
		RunStateError("分配地图阻挡点内存失败！地图ID：%d", mapid);
		return false;
	}
	memset(barinfo, 0, m_Width * m_Height * sizeof(bool));

	pinfo = pinfo->FirstChildElement("bar");

	while (pinfo)
	{
		int row = 0;
		int col = 0;

		if (pinfo->QueryIntAttribute("row", &row) != XML_SUCCESS)
		{
			RunStateError("没有找到字段： 'row'");
			delete []barinfo;
			return false;
		}

		if (row < 0 || row > m_Width)
		{
			RunStateError("地图阻挡点行数 < 0 或者行数大于地图宽 ,地图ID：%d ", mapid);
			delete []barinfo;
			return false;
		}

		if (pinfo->QueryIntAttribute("col", &col) != XML_SUCCESS)
		{
			RunStateError("没有找到字段： 'col'");
			delete []barinfo;
			return false;
		}

		if (col < 0 || col > m_Height)
		{
			RunStateError("地图阻挡点列数 < 0 或者列数大于地图高 ,地图ID：%d ", mapid);
			delete []barinfo;
			return false;
		}

		barinfo[row * col] = true;
		pinfo = pinfo->NextSiblingElement("bar");
	}

	m_BarInfo = barinfo;
	return true;
}

void CMapInfo::Destroy()
{
	if (m_BarInfo)
	{
		delete[]m_BarInfo;
		m_BarInfo = nullptr;
	}
}

void CMapInfo::SetMapBirthPoint(const float &x, const float &y, const float &z)
{
	m_BirthPoint_X = x;
	m_BirthPoint_Y = y;
	m_BirthPoint_Z = z;
}

void CMapInfo::GetMapBirthPoint(float &x, float &y, float &z)
{
	x = m_BirthPoint_X;
	y = m_BirthPoint_Y;
	z = m_BirthPoint_Z;
}

void CMapInfo::GetMapWidthAndHeight(int32 &x, int32 &y)
{
	x = m_Width;
	y = m_Height;
}

bool CMapInfo::bCanMove(int32 x, int32 y, int32 z)
{
	if (x >= 0 && x < m_Width)
	{
		if (y >= 0 && y < m_Height)
		{
			if (m_BarInfo)
			{
				return !m_BarInfo[x * y];
			}
		}
	}
	return false;
}
