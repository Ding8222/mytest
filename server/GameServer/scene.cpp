#include "stdfx.h"
#include "scene.h"
#include "BaseObj.h"
#include "mapinfo.h"

CScene::CScene()
{
	m_MapID = 0;
	m_Width = 0;
	m_Height = 0;
	m_BirthPoint_X = 0;
	m_BirthPoint_Y = 0;
	m_TempID = 0;
	m_Barinfo = nullptr;
	m_MapInfo = nullptr;
	m_Cookie = nullptr;
	m_Space = nullptr;
	m_bMessage = false;
	m_ObjMap.clear();
}

CScene::~CScene()
{
	if (m_MapInfo)
	{
		delete m_MapInfo;
		m_MapInfo = nullptr;
	}
	m_MapInfo = nullptr;

	if (m_Space)
	{
		aoi_release(m_Space);
		m_Space = nullptr;
	}
	if (m_Cookie)
	{
		free(m_Cookie);
	}
}

bool CScene::Init(CMapInfo * _mapinfo, aoi_space * space, alloc_cookie * cookie)
{
	if (!_mapinfo || !space)
		return false;

	m_MapInfo = _mapinfo;
	m_Space = space;
	m_Cookie = cookie;
	m_Barinfo = m_MapInfo->GetBarInfo();

	m_MapID = _mapinfo->GetMapID();
	_mapinfo->GetMapBirthPoint(m_BirthPoint_X, m_BirthPoint_Y, m_BirthPoint_Z);
	_mapinfo->GetMapWidthAndHeight(m_Width, m_Height);

	if (m_Width <= 0 && m_Height <= 0)
	{
		log_error("CScene get map width and height failed!");
		return false;
	}
	return true;
}

bool CScene::AddObj(CBaseObj * obj)
{
	if (!obj)
		return false;
	
	m_ObjMap[obj->GetTempID()] = obj;

	char *mode = "wm";
	float pos[3];
	obj->GetNowPos(pos[0], pos[1], pos[2]);
	Update(obj->GetTempID(), mode, pos);

	return true;
}

bool CScene::DelObj(CBaseObj * obj)
{
	if (!obj)
		return false;
	
	char *mode = "d";
	float pos[3];
	obj->GetNowPos(pos[0], pos[1], pos[2]);
	Update(obj->GetTempID(), mode, pos);

	m_ObjMap.erase(obj->GetTempID());

	return true;
}

bool CScene::bCanMove(int x, int y, int z)
{
	if (x >= 0 && x < m_Width)
	{
		if (y >= 0 && y < m_Height)
		{
			if (m_Barinfo)
			{
				return !m_Barinfo[x * y];
			}
		}
	}
	return false;
}

bool CScene::MoveTo(CBaseObj * obj, float x, float y, float z)
{
	if (obj)
	{
		if (bCanMove(x, y, z))
		{
			obj->SetNowPos(x, y, z);
			char *mode = "wm";
			float pos[3];
			obj->GetNowPos(pos[0], pos[1], pos[2]);
			Update(obj->GetTempID(), mode, pos);
			return true;
		}
	}

	return false;
}

static void callbackmessage(void *ud, uint32 watcher, uint32 marker) 
{
	CScene* sc = (CScene*)ud;
	CBaseObj * p1 = sc->GetObj(watcher);
	if (p1)
	{
		CBaseObj * p2 = sc->GetObj(marker);
		p1->AddToAoiList(p2);
	}
}

void CScene::Message()
{
	aoi_message(m_Space, callbackmessage, this);
}

void CScene::Update(uint32 id, const char * mode,float pos[3])
{
	aoi_update(m_Space, id, mode, pos);
	m_bMessage = true;
}

void CScene::Run()
{
	if (m_bMessage)
	{
		Message();
		m_bMessage = false;
	}

	for (auto &i : m_ObjMap)
	{
		//i.second->Run();
	}
}

CBaseObj * CScene::GetObj(uint32 id)
{
	auto iter = m_ObjMap.find(id);
	if (iter != m_ObjMap.end())
		return iter->second;

	return nullptr;
}