#include"mapinfo.h"
#include"tinyxml2.h"
#include"log.h"

using namespace tinyxml2;

mapinfo::mapinfo()
{
	m_mapid = 0;
	m_width = 0;
	m_height = 0;
	m_birth_point_x = 0;
	m_birth_point_y = 0;
	m_barinfo = nullptr;
	m_bar_filename.clear();
}

mapinfo::~mapinfo()
{
	if (m_barinfo)
	{
		delete m_barinfo;
		m_barinfo = nullptr;
	}
	m_barinfo = nullptr;
	m_bar_filename.clear();
	m_mapid = 0;
	m_width = 0;
	m_height = 0;
}

bool mapinfo::init(int mapid, std::string bar_filename)
{
	// 设置读取的路径
	m_mapid = mapid;
	m_bar_filename = bar_filename;
	return true;
}

void mapinfo::getmapbirthpoint(int &x, int &y)
{
	x = m_birth_point_x;
	y = m_birth_point_y;
}
int mapinfo::getmapid()
{
	return m_mapid;
}
void mapinfo::getmapwidthandheight(int &x, int &y)
{
	x = m_width;
	y = m_height;
}