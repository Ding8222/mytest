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
	bool loadbar(mapinfo* map); //�������е�ͼ���赲��
	mapinfo *getmapinfo(int mapid);
	std::unordered_map<int, mapinfo*>* getmaplist();
private:

	std::unordered_map<int, mapinfo*> m_maplist;//���е�map��Ϣ
};