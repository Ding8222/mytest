#include "platform_config.h"
#include "BaseObj.h"
#include "Scene.h"
#include "MapInfo.h"
#include "idmgr.c"
#include "serverlog.h"
#include "Utilities.h"
#include "MonsterCreator.h"
#include "NPCCreator.h"

CScene::CScene()
{
	m_MapInfo = nullptr;
	m_Cookie = nullptr;
	m_Space = nullptr;
	m_bMessage = false;
	m_IDPool = nullptr;
	m_ObjMap.clear();
	m_ObjSet.clear();
	m_NPCList.clear();
	m_WaitRemoveNPC.clear();
	m_MonsterList.clear();
	m_WaitRemoveMonster.clear();
}

CScene::~CScene()
{
	Destroy();
}

static void *my_alloc(void * ud, void *ptr, size_t sz) {
	struct alloc_cookie * cookie = (struct alloc_cookie *)ud;
	if (ptr == NULL) {
		void *p = nullptr;
		try {
			p = malloc(sz);
		}
		catch (std::bad_alloc &) {
			p = nullptr;
		}
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

bool CScene::Init(CMapInfo * _mapinfo)
{
	if (!_mapinfo)
		return false;
	
	m_MapInfo = _mapinfo;

	int32 width = 0;
	int32 height = 0;
	m_MapInfo->GetMapWidthAndHeight(width, height);
	if (m_MapInfo->GetMapID() <= 0 || width <= 0 || height <= 0)
	{
		RunStateError("场景[%d]获取地图宽[%d]、高[%d]失败!", m_MapInfo->GetMapID(), width, height);
		return false;
	}

	m_IDPool = idmgr_create(SCENE_ID_MAX + 1, CLIENT_ID_DELAY_TIME);
	if (!m_IDPool)
	{
		RunStateError("创建IDMgr失败!");
		return false;
	}

	alloc_cookie* cookie = nullptr;
	try {
		cookie = new alloc_cookie;
	}
	catch (std::bad_alloc &) {
		cookie = nullptr;
	}
	if (cookie)
	{
		memset(cookie, 0, sizeof(alloc_cookie));
		aoi_space * space = aoi_create(my_alloc, cookie);
		if (space)
		{
			m_Space = space;
			m_Cookie = cookie;
		}
		else
		{
			delete cookie;
			cookie = nullptr;
			idmgr_release(m_IDPool);
		}
	}
	else
		idmgr_release(m_IDPool);

	m_ObjSet.resize(SCENE_ID_MAX + 1, nullptr);

	return true;
}

void CScene::Run()
{
	if (m_bMessage)
	{
		Message();
		m_bMessage = false;
	}

	ProcessAllMonster();
	ProcessAllNPC();

	CheckAndRemoveMonster();
	CheckAndRemoveNPC();
}

void CScene::Destroy()
{
	DelAllNPC();
	DelAllMonster();

	if (m_Space)
	{
		aoi_release(m_Space);
		m_Space = nullptr;
	}

	if (m_Cookie)
	{
		delete m_Cookie;
		m_Cookie = nullptr;
	}

	if (m_IDPool)
	{
		idmgr_release(m_IDPool);
		m_IDPool = nullptr;
	}

	m_ObjMap.clear();
	m_ObjSet.clear();
}

bool CScene::AddObj(CBaseObj * obj)
{
	if (!FuncUti::isValidCret(obj))
		return false;
	
	CScene *OldScene = obj->GetScene();
	assert(!OldScene);
	if (OldScene)
	{
		if (!OldScene->DelObj(obj))
		{
			RunStateError("对象%d从原地图[%d]移除失败!", obj->GetTempID(), OldScene->GetMapID());
			return false;
		}
	}

	int32 id = idmgr_allocid(m_IDPool);
	if (id <= 0)
	{
		RunStateError("为新对象分配ID失败!, id:%d", id);
		return false;
	}
	obj->SetTempID(id);
	obj->SetScene(this);
	m_ObjMap[obj->GetTempID()] = obj;

	assert(m_ObjSet[id] == nullptr);
	m_ObjSet[id] = obj;

	return true;
}

bool CScene::DelObj(CBaseObj * obj)
{
	if (!FuncUti::isValidCret(obj))
		return false;

	int32 id = obj->GetTempID();
	if (id <= 0 || id >= static_cast<int32> (m_ObjSet.size()))
	{
		RunStateError("要释放的CBaseObj的ID错误!");
		return false;
	}

	m_ObjSet[id] = nullptr;

	if (!idmgr_freeid(m_IDPool, obj->GetTempID()))
	{
		RunStateError("释放ID错误, ID:%d", id);
	}

	obj->LeaveAoi();
	obj->SetTempID(0);
	m_ObjMap.erase(obj->GetTempID());
	return true;
}

CBaseObj * CScene::GetObj(int32 id)
{
	if (id <= 0 || id >= static_cast<int32> (m_ObjSet.size()))
		return nullptr;

	return m_ObjSet[id];
}
bool CScene::MoveTo(CBaseObj * obj, float x, float y, float z)
{
	if (FuncUti::isValidCret(obj))
	{
		if (m_MapInfo->bCanMove(static_cast<int>(floor(x)), static_cast<int>(floor(y)), static_cast<int>(floor(z))))
		{
			obj->SetNowPos(x, y, z);
			Update(obj->GetTempID(), obj->GetAoiMode(), obj->GetNowPos());
			return true;
		}
	}

	return false;
}

// 添加NPC至场景
bool CScene::AddNPC(int32 npcid, float x, float y, float z)
{
	CNPC *npc = CNPCCreator::CreateNPC();
	if (!npc)
	{
		RunStateError("创建CNPC失败！");
		return false;
	}

	if (!npc->Init(npcid))
	{
		RunStateError("NPC：%d 初始化失败！", npcid);
		CNPCCreator::ReleaseNPC(npc);
		return false;
	}

	if (!AddObj(npc))
	{
		RunStateError("NPC：%d 添加到地图：%d 失败！", npcid, GetMapID());
		npc->Die();
		CNPCCreator::ReleaseNPC(npc);
		return false;
	}

	if (!npc->MoveTo(x, y, z))
	{
		RunStateError("NPC：%d 初始化坐标（%d,%d,%d）失败！", npcid, x, y, z);
		npc->Die();
		if (!DelObj(npc))
		{
			RunStateError("NPC：%d 移出地图：%d 失败！", npcid, GetMapID());
		}
		CNPCCreator::ReleaseNPC(npc);
		return false;
	}

	m_NPCList.push_back(npc);

	return true;
}

void CScene::ProcessAllNPC()
{
	std::list<CNPC*>::iterator iter, tempiter;
	for (iter = m_NPCList.begin(); iter != m_NPCList.end();)
	{
		tempiter = iter;
		++iter;

		if ((*tempiter)->IsWaitRemove())
		{
			m_WaitRemoveNPC.push_back(*tempiter);
			m_NPCList.erase(tempiter);
			continue;
		}

		(*tempiter)->Run();
	}
}

void CScene::CheckAndRemoveNPC()
{
	CNPC *npc;
	while (!m_WaitRemoveNPC.empty())
	{
		npc = m_WaitRemoveNPC.front();
		if (npc && !npc->CanRemove(g_currenttime))
			break;

		CNPCCreator::ReleaseNPC(npc);
		m_WaitRemoveNPC.pop_front();
	}
}

void CScene::DelAllNPC()
{
	for (std::list<CNPC *>::iterator itr = m_NPCList.begin(); itr != m_NPCList.end(); ++itr)
	{
		(*itr)->Die();
		CNPCCreator::ReleaseNPC(*itr);
	}
	m_NPCList.clear();

	for (std::list<CNPC *>::iterator itr = m_WaitRemoveNPC.begin(); itr != m_WaitRemoveNPC.end(); ++itr)
	{
		CNPCCreator::ReleaseNPC(*itr);
	}
	m_WaitRemoveNPC.clear();
}


// 添加Monster至场景
bool CScene::AddMonster(int32 monsterid, float x, float y, float z)
{
	CMonster *monster = CMonsterCreator::CreateMonster();
	if (!monster)
	{
		RunStateError("创建CMonster失败！");
		return false;
	}

	if (!monster->Init(monsterid))
	{
		RunStateError("Monster：%d 初始化失败！", monsterid);
		CMonsterCreator::ReleaseMonster(monster);
		return false;
	}

	if (!AddObj(monster))
	{
		RunStateError("Monster：%d 添加到地图：%d 失败！", monsterid, GetMapID());
		monster->Die();
		CMonsterCreator::ReleaseMonster(monster);
		return false;
	}

	if (!monster->MoveTo(x, y, z))
	{
		RunStateError("Monster：%d 初始化坐标（%d,%d,%d）失败！", monsterid, x, y, z);
		monster->Die();
		CMonsterCreator::ReleaseMonster(monster);
		return false;
	}

	m_MonsterList.push_back(monster);

	return true;
}

void CScene::ProcessAllMonster()
{
	std::list<CMonster *>::iterator iter, tempiter;
	for (iter = m_MonsterList.begin(); iter != m_MonsterList.end();)
	{
		tempiter = iter;
		++iter;

		// 不能复活，而且等待被移除
		if (!(*tempiter)->bCanRelive() && (*tempiter)->IsWaitRemove())
		{
			m_WaitRemoveMonster.push_back(*tempiter);
			m_MonsterList.erase(tempiter);
			continue;
		}

		(*tempiter)->Run();
	}
}

void CScene::CheckAndRemoveMonster()
{
	CMonster *monster;
	while (!m_WaitRemoveMonster.empty())
	{
		monster = m_WaitRemoveMonster.front();
		if (monster && !monster->CanRemove(g_currenttime))
			break;

		CMonsterCreator::ReleaseMonster(monster);
		m_WaitRemoveMonster.pop_front();
	}
}

void CScene::DelAllMonster()
{
	for (std::list<CMonster *>::iterator itr = m_MonsterList.begin(); itr != m_MonsterList.end(); ++itr)
	{
		(*itr)->Die();
		CMonsterCreator::ReleaseMonster(*itr);
	}
	m_MonsterList.clear();

	for (std::list<CMonster *>::iterator itr = m_WaitRemoveMonster.begin(); itr != m_WaitRemoveMonster.end(); ++itr)
	{
		CMonsterCreator::ReleaseMonster(*itr);
	}
	m_WaitRemoveMonster.clear();
}

static void callbackmessage(void *ud, uint32 watcher, uint32 marker) 
{
	CScene *sc = (CScene *)ud;
	if (sc)
	{
		CBaseObj *p1 = sc->GetObj(watcher);
		CBaseObj *p2 = sc->GetObj(marker);
		if (FuncUti::isValidCret(p1))
		{
			if (FuncUti::isValidCret(p2))
			{
				p1->AddToAoiList(p2);
				// 非玩家，需要帮忙加一下
				if (!p2->IsPlayer())
					p2->AddToAoiList(p1);
			}
		}
	}
}

void CScene::Message()
{
	aoi_message(m_Space, callbackmessage, this);
}

void CScene::Update(uint32 id, const char *mode,float pos[3])
{
	aoi_update(m_Space, id, mode, pos);
	m_bMessage = true;
}
