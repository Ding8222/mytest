#include"MapInfo.h"
#include"tinyxml2.h"
#include"log.h"

using namespace tinyxml2;

CMapInfo::CMapInfo()
{
	m_Mapid = 0;
	m_Width = 0;
	m_Height = 0;
	m_BirthPoint_X = 0;
	m_BirthPoint_Y = 0;
	m_BirthPoint_Z = 0;
	m_BarInfo = nullptr;
}

CMapInfo::~CMapInfo()
{
	Destroy();
}

bool CMapInfo::Init(int mapid, const char *bar_filename)
{
	if (!bar_filename)
	{
		log_error("地图配置阻挡点文件路径不存在！");
		return false;
	}

	// 设置读取的路径
	m_Mapid = mapid;

	XMLDocument doc;
	if (doc.LoadFile(bar_filename) != XML_SUCCESS)
	{
		log_error("加载 %s 失败!", bar_filename);
		return false;
	}

	// 添加地图阻挡点信息

	XMLElement *pinfo = doc.FirstChildElement("bar_map");
	if (!pinfo)
	{
		log_error("没有找到字段： 'bar_map'");
		return false;
	}

	if (pinfo->QueryIntAttribute("width", &m_Width) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'width'");
		return false;
	}

	if (m_Width <= 0)
	{
		log_error("地图宽 <= 0 ,地图ID：%d ", mapid);
		return false;
	}

	if (pinfo->QueryIntAttribute("height", &m_Height) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'height'");
		return false;
	}

	if (m_Height <= 0)
	{
		log_error("地图宽 <= 0 ,地图ID：%d ", mapid);
		return false;
	}

	bool* barinfo = new bool[m_Width * m_Height];

	if (!barinfo)
	{
		log_error("分配地图阻挡点内存失败！地图ID：%d", mapid);
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
			log_error("没有找到字段： 'row'");
			delete(barinfo);
			return false;
		}

		if (row < 0 || row > m_Width)
		{
			log_error("地图阻挡点行数 < 0 或者行数大于地图宽 ,地图ID：%d ", mapid);
			delete(barinfo);
			return false;
		}

		if (pinfo->QueryIntAttribute("col", &col) != XML_SUCCESS)
		{
			log_error("没有找到字段： 'col'");
			delete(barinfo);
			return false;
		}

		if (col < 0 || col > m_Height)
		{
			log_error("地图阻挡点列数 < 0 或者列数大于地图高 ,地图ID：%d ", mapid);
			delete(barinfo);
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
		delete m_BarInfo;
	}
	m_BarInfo = nullptr;
}

void CMapInfo::GetMapBirthPoint(int &x, int &y, int &z)
{
	x = m_BirthPoint_X;
	y = m_BirthPoint_Y;
	z = m_BirthPoint_Z;
}
int CMapInfo::GetMapID()
{
	return m_Mapid;
}
void CMapInfo::GetMapWidthAndHeight(int &x, int &y)
{
	x = m_Width;
	y = m_Height;
}