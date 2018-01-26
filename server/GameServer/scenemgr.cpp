#include "stdfx.h"
#include "scenemgr.h"
#include "mapconfig.h"
#include "scene.h"

CScenemgr::CScenemgr()
{
	m_SceneList.clear();
}

CScenemgr::~CScenemgr()
{
	m_SceneList.clear();
}

bool CScenemgr::Init()
{
	const std::unordered_map<int, CMapInfo*> maplist = CMapConfig::Instance().GetMapList();

	for (auto &iter : maplist)
	{
		if (!LoadScene(iter.second))
		{
			log_error("load scene error ,mapid: %d", iter.first);
			return false;
		}
	}

	return true;
}

void CScenemgr::Destroy()
{

}

void CScenemgr::Run()
{
	for (auto &i : m_SceneList)
	{
		i.second->Run();
	}
}

bool CScenemgr::LoadScene(CMapInfo* mapconfig)
{
	if (!mapconfig)
		return false;

	CScene * m_scene = CreateScene(mapconfig);
	if (!m_scene)
		return false;

	m_SceneList[m_scene->GetMapID()] = m_scene;

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

CScene *CScenemgr::CreateScene(CMapInfo* mapconfig)
{
	if (!mapconfig)
		return nullptr;

	CScene * m_scene = new CScene;
	alloc_cookie* cookie = (struct alloc_cookie * )malloc(sizeof(struct alloc_cookie));
	if (cookie)
	{
		memset(cookie, 0, sizeof(alloc_cookie));
		aoi_space * space = aoi_create(my_alloc, cookie);
		if (m_scene)
		{
			m_scene->Init(mapconfig, space, cookie);
			return m_scene;
		}
	}

	return nullptr;
}

CScene *CScenemgr::GetScene(int mapid)
{
	auto iter = m_SceneList.find(mapid);
	if (iter != m_SceneList.end())
		return iter->second;

	return nullptr;
}