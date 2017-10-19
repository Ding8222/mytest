#pragma once
#include<string>
#include<unordered_map>

class client;
class scene;

enum ePlayerPos
{
	EPP_X = 0,
	EPP_Y = 1,
	EPP_Z = 2,
	EPP_MAX = 3,
};

class playerobj
{
public:
	playerobj();
	~playerobj();

	bool load(std::string name, scene *_scene, client* _client);
	bool moveto(float &x, float &y, float &z);
	void getnowpos(float &x, float &y, float &z);
	void setnowpos(const float &x, const float &y, const float &z);
	void addtoaoilist(playerobj * p);
	std::string getname() { return m_name; }
	inline uint32_t gettempid() {
		return m_tempid;
	}

	inline void setteampid(uint32_t id)
	{
		m_tempid = id;
	}
private:
	int m_now_mapid;
	float m_now_pos[EPP_MAX];
	int m_tempid;

	client *m_client;
	scene *m_scene;
	std::string m_name;
	std::unordered_map<uint32_t, playerobj *> m_aoilist;
};