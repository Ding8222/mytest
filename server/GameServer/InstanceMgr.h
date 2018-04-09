/*
* 副本管理
* Copyright (C) ddl
* 2018
*/
#pragma once

struct idmgr;
class CInstance;
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
	bool AddNPC(int32 npcid, int32 mapid, float x, float y, float z);
	bool AddMonster(int32 monsterid, int32 mapid, float x, float y, float z);
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