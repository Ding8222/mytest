#include "stdfx.h"

scenemgr::scenemgr()
{
	m_scenelist.clear();
}

scenemgr::~scenemgr()
{
	m_scenelist.clear();
}

bool scenemgr::init()
{
	std::unordered_map<int, mapinfo*> * maplist = mapconfig::Instance().getmaplist();
	if (!maplist)
		return false;

	for (auto &iter : *maplist)
	{
		if (!loadscene(iter.second))
		{
			log_error("load scene error ,mapid: %d", iter.first);
			return false;
		}
	}

	return true;
}

void scenemgr::run()
{
	for (auto &i : m_scenelist)
	{
		i.second->run();
	}
}

bool scenemgr::loadscene(mapinfo* mapconfig)
{
	if (!mapconfig)
		return false;

	scene * m_scene = createscene(mapconfig);
	if (!m_scene)
		return false;

	m_scenelist[m_scene->getmapid()] = m_scene;

	return true;
}

static void *my_alloc(void * ud, void *ptr, size_t sz) {
	struct alloc_cookie * cookie = (struct alloc_cookie *)ud;
	if (ptr == NULL) {
		void *p = malloc(sz);
		++cookie->count;
		cookie->current += sz;
		if (cookie->max < cookie->current) {
			cookie->max = cookie->current;
		}
		//		printf("%p + %u\n",p, sz);
		return p;
	}
	--cookie->count;
	cookie->current -= sz;
	//	printf("%p - %u \n",ptr, sz);
	free(ptr);
	return NULL;
}

scene *scenemgr::createscene(mapinfo* mapconfig)
{
	if (!mapconfig)
		return nullptr;

	scene * m_scene = new scene;
	alloc_cookie* cookie = (struct alloc_cookie * )malloc(sizeof(struct alloc_cookie));
	if (cookie)
	{
		memset(cookie, 0, sizeof(alloc_cookie));
		aoi_space * space = aoi_create(my_alloc, cookie);
		if (m_scene)
		{
			m_scene->init(mapconfig, space, cookie);
			return m_scene;
		}
	}

	return nullptr;
}

scene *scenemgr::getscene(int mapid)
{
	auto iter = m_scenelist.find(mapid);
	if (iter != m_scenelist.end())
		return iter->second;

	return nullptr;
}