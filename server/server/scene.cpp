#include "stdfx.h"

scene::scene()
{
	m_mapid = 0;
	m_width = 0;
	m_height = 0;
	m_birth_point_x = 0;
	m_birth_point_y = 0;
	m_tempid = 0;
	m_barinfo = nullptr;
	m_mapinfo = nullptr;
	m_cookie = nullptr;
	m_space = nullptr;
	m_bMessage = false;
	m_playermap.clear();
}

scene::~scene()
{
	if (m_mapinfo)
	{
		delete m_mapinfo;
		m_mapinfo = nullptr;
	}
	m_mapinfo = nullptr;

	if (m_space)
	{
		aoi_release(m_space);
		m_space = nullptr;
	}
	if (m_cookie)
	{
		free(m_cookie);
	}
}

bool scene::init(mapinfo * _mapinfo, aoi_space * space, alloc_cookie * cookie)
{
	if (!_mapinfo || !space)
		return false;

	m_mapinfo = _mapinfo;
	m_space = space;
	m_cookie = cookie;
	m_barinfo = m_mapinfo->getbarinfo();

	m_mapid = _mapinfo->getmapid();
	_mapinfo->getmapbirthpoint(m_birth_point_x, m_birth_point_y);
	_mapinfo->getmapwidthandheight(m_width, m_height);

	if (m_width <= 0 && m_height <= 0)
	{
		log_error("scene get map width and height failed!");
		return false;
	}
	return true;
}

bool scene::obj_enter(playerobj * obj)
{
	if (!obj)
		return false;
	
	obj->setteampid(gettempid());
	m_playermap[obj->gettempid()] = obj;

	char *mode = "wm";
	float pos[3];
	obj->getnowpos(pos[0], pos[1], pos[2]);
	update(obj->gettempid(), mode, pos);

	return true;
}

bool scene::canmove(int x, int y, int z)
{
	if (x >= 0 && x < m_width)
	{
		if (y >= 0 && y < m_height)
		{
			if (m_barinfo)
			{
				return !m_barinfo[x * y];
			}
		}
	}
	return false;
}

bool scene::moveto(playerobj * obj, float x, float y, float z)
{
	if (obj)
	{
		if (canmove(x, y, z))
		{
			obj->setnowpos(x, y, z);
			char *mode = "wm";
			float pos[3];
			obj->getnowpos(pos[0], pos[1], pos[2]);
			update(obj->gettempid(), mode, pos);
			return true;
		}
	}

	return false;
}

static void callbackmessage(void *ud, uint32_t watcher, uint32_t marker) 
{
	scene* sc = (scene*)ud;
	playerobj * p1 = sc->getobj(watcher);
	if (p1)
	{
		playerobj * p2 = sc->getobj(marker);
		p1->addtoaoilist(p2);
	}
}

void scene::message()
{
	aoi_message(m_space, callbackmessage, this);
}

void scene::update(uint32_t id, const char * mode,float pos[3])
{
	aoi_update(m_space, id, mode, pos);
	m_bMessage = true;
}

void scene::run()
{
	if (m_bMessage)
	{
		message();
		m_bMessage = false;
	}
}

playerobj * scene::getobj(uint32_t id)
{
	auto iter = m_playermap.find(id);
	if (iter != m_playermap.end())
		return iter->second;

	return nullptr;
}