/*
* 场景数据
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>
extern "C"
{
#include "aoi.h"
}
#include "platform_config.h"
#include "MapInfo.h"
#include "Monster.h"
#include "NPC.h"

class CMapInfo;
class CBaseObj;
struct idmgr;

struct alloc_cookie {
	alloc_cookie()
	{
		count = 0;
		max = 0;
		current = 0;
	}
	~alloc_cookie()
	{
		count = 0;
		max = 0;
		current = 0;
	}
	int32 count;
	int32 max;
	int32 current;
};

class CScene
{
public:
	CScene();
	~CScene();

	bool Init(CMapInfo * _mapinfo);
	void Destroy();
	void Run();

	// 进入场景
	bool AddObj(CBaseObj * obj);
	// 退出场景
	bool DelObj(CBaseObj * obj);
	// 获取场景中的对象
	CBaseObj * GetObj(int32 id);
	// 移动至某个点
	bool MoveTo(CBaseObj * obj, float x, float y, float z);
public:
	// 添加NPC至场景
	bool AddNPC(int32 npcid, float x, float y, float z);
private:
	// 处理所有NPC
	void ProcessAllNPC();
	// 检查并真正移除NPC
	void CheckAndRemoveNPC();
	// 真正移除所有的NPC
	void DelAllNPC();

	std::list<CNPC *> m_NPCList;
	std::list<CNPC *> m_WaitRemoveNPC;
public:
	// 添加Monster至场景
	bool AddMonster(int32 monsterid, float x, float y, float z, bool relive = false, int32 relivecd = 0);
private:
	// 处理所有Monster
	void ProcessAllMonster();
	// 检查并真正移除Monster
	void CheckAndRemoveMonster();
	// 真正移除所有的Monster
	void DelAllMonster();

	std::list<CMonster *> m_MonsterList;
	std::list<CMonster *> m_WaitRemoveMonster;
public:

	// Aoi Run
	void Message();
	// 更新对象在Aoi中的位置
	void Update(uint32 id, const char * mode, float pos[3]);
private:

	// Aoi内存分配信息
	struct alloc_cookie* m_Cookie;
	// Aoi 空间
	struct aoi_space * m_Space;
	// 是否更新过Aoi
	bool m_bMessage;
public:

	// 获取当前场景所属的MapID
	int32 GetMapID() { return m_MapInfo->GetMapID(); }
	int32 GetMapType() { return m_MapInfo->GetMapType(); }
private:

	// 地图配置信息
	CMapInfo * m_MapInfo;

	std::unordered_map<uint32, CBaseObj *> m_ObjMap;
	std::vector<CBaseObj *> m_ObjSet;
	idmgr *m_IDPool;
};