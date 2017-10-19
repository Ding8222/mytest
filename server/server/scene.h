#pragma once
extern "C"
{
#include "aoi.h"
}

class mapinfo;

struct alloc_cookie {
	int count;
	int max;
	int current;
};

class scene
{
public:
	scene();
	~scene();

	bool init(mapinfo * _mapinfo, aoi_space * space, alloc_cookie * cookie);
	void run();

	bool obj_enter(playerobj * obj);
	playerobj * getobj(uint32_t id);

	bool canmove(int x, int y, int z);
	bool moveto(playerobj * obj, float x, float y, float z);

	void message();
	void update(uint32_t id, const char * mode, float pos[3]);

	inline uint32_t gettempid() { return ++m_tempid; };
	inline int getmapid() { return m_mapid; }
private:
	int m_mapid;
	int m_width;
	int m_height;

	int m_birth_point_x;
	int m_birth_point_y;

	uint32_t m_tempid;

	bool *m_barinfo; //×èµ²ÐÅÏ¢
	mapinfo *m_mapinfo;
	struct alloc_cookie* m_cookie;
	struct aoi_space * m_space;
	bool m_bMessage;
	std::unordered_map<uint32_t, playerobj *> m_playermap;
};