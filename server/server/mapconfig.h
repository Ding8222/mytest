#pragma once
#include<unordered_map>
#include "mapinfo.h"

class mapconfig
{
public:
	mapconfig();
	~mapconfig();

	bool init();
private:

	std::unordered_map<int, mapinfo*> m_maplist;//���е�map��Ϣ
};