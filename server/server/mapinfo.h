/*
*
*	地图数据
*
*/

/*
	表中读到的地图信息
*/

#pragma once
#include "string"

class mapinfo
{
public:
	mapinfo();
	~mapinfo();

	bool init(int mapid, std::string bar_filename);
	void getmapbirthpoint(int &x, int &y); //获取出生点
	int  getmapid(); //获取地图id
	void getmapwidthandheight(int &x, int &y); //获取地图宽高
	inline const std::string &getbarfilename() { return m_bar_filename; }
	void setmapbarinfo(int width, int height, bool* barinfo) {
		m_width = width; m_height = height; m_barinfo = barinfo;
	};
	bool *getbarinfo() { return m_barinfo; }
private:
	int m_mapid;
	int m_width;
	int m_height;

	int m_birth_point_x;
	int m_birth_point_y;

	bool *m_barinfo; //阻挡信息
	std::string m_bar_filename;
};