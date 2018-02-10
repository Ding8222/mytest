/*
* 副本管理
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "Instance.h"

struct idmgr;
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
	int AddInstance(int instancebaseid);
	CInstance *FindInstance(int instanceid);
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