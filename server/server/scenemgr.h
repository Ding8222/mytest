#pragma once
#include"scenemgr.h"
#include<unordered_map>

class scene;
class mapinfo;

class scenemgr
{
public:
	scenemgr();
	~scenemgr();

	static scenemgr &Instance()
	{
		static scenemgr m;
		return m;
	}

	bool init();
	void run();
	bool loadscene(mapinfo* mapconfig);
	scene *createscene(mapinfo* mapconfig);
	scene *getscene(int mapid);
private:

	std::unordered_map<int ,scene *> m_scenelist;
};