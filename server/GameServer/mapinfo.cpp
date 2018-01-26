#include"mapinfo.h"
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
	s_BarFileName.clear();
}

CMapInfo::~CMapInfo()
{
	m_Mapid = 0;
	m_Width = 0;
	m_Height = 0;
	m_BirthPoint_X = 0;
	m_BirthPoint_Y = 0;
	m_BirthPoint_Z = 0;
	if (m_BarInfo)
	{
		delete m_BarInfo;
		m_BarInfo = nullptr;
	}
	m_BarInfo = nullptr;
	s_BarFileName.clear();
}

bool CMapInfo::Init(int mapid, std::string bar_filename)
{
	// 设置读取的路径
	m_Mapid = mapid;
	s_BarFileName = bar_filename;
	return true;
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