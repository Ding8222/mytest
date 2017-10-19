#include "stdfx.h"

playerobj::playerobj()
{
	m_now_mapid = 0;
	memset(m_now_pos, 0, sizeof(float) * EPP_MAX);
	m_name.clear();
	m_scene = nullptr;
}

playerobj::~playerobj()
{
	m_now_mapid = 0;
	memset(m_now_pos, 0, sizeof(float) * EPP_MAX);
	m_name.clear();
	m_scene = nullptr;
}

bool playerobj::load(std::string name, scene* _scene, client* _client)
{
	if (!_scene || !_client)
		return false;

	m_now_mapid = _scene->getmapid();
	m_name = name;
	m_scene = _scene;
	m_client = _client;

	_scene->obj_enter(this);

	return true;
}

bool playerobj::moveto(float &x, float &y, float &z)
{
	if (m_scene)
	{
		if (m_scene->moveto(this, x, y, z))
		{
			return true;
		}
	}

	return false;
}

void playerobj::getnowpos(float &x, float &y, float &z)
{
	x = m_now_pos[EPP_X];
	y = m_now_pos[EPP_Y];
	z = m_now_pos[EPP_Z];
}

void playerobj::setnowpos(const float &x, const float &y, const float &z)
{
	m_now_pos[EPP_X] = x;
	m_now_pos[EPP_Y] = y;
	m_now_pos[EPP_Z] = z;
}

void playerobj::addtoaoilist(playerobj * p)
{
	if (p)
	{
		m_aoilist[p->gettempid()] = p;
		MessagePack msg;
		msg.Reset();
		msg.SetType(MSG_ENTER);
		msg.PushString(p->getname().c_str());
		m_client->SendMsg(&msg);
	}
}