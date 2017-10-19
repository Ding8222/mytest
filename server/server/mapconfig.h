#pragma once
#include<unordered_map>
#include "mapinfo.h"

class mapconfig
{
public:
	mapconfig();
	~mapconfig();

	static mapconfig &Instance()
	{
		static mapconfig m;
		return m;
	}

	bool init();
	bool loadbar(mapinfo* map); //加载所有地图的阻挡点
	mapinfo *getmapinfo(int mapid);
	std::unordered_map<int, mapinfo*>* getmaplist();
private:

	std::unordered_map<int, mapinfo*> m_maplist;//所有的map信息
};