/*
* 副本管理
* Copyright (C) ddl
* 2018
*/
#pragma once

struct idmgr;
class CBaseObj;
class CInstance;
#define InstanceMgr CInstanceMgr::Instance()
class CInstanceMgr
{
public:
	CInstanceMgr();
	~CInstanceMgr();

	static CInstanceMgr &Instance()
	{
		static CInstanceMgr m;
		return m;
	}

	bool Init();
	void Run();
	void Destroy();

	// 传入副本基本ID，返回副本实例ID
	int32 AddInstance(int32 instancebaseid);
	// 根据副本实例ID查找副本
	CInstance *FindInstance(int32 instanceid);
	// 添加NPC
	bool AddNPC(int32 npcid, int32 instanceid, float x, float y, float z);
	// 添加怪物
	bool AddMonster(int32 monsterid, int32 instanceid, float x, float y, float z);
	// 进入副本
	bool EnterInstance(CBaseObj * obj, int32 instanceid);
private:
	void ProcessAllInstance();
	void CheckAndRemove();
	void ReleaseInstanceAndID(CInstance *scene);

private:

	std::list<CInstance *> m_InstanceList;
	std::list<CInstance *> m_WaitRemove;
	std::vector<CInstance *> m_InstanceSet;
	idmgr *m_IDPool;
};