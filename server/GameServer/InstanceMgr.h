/*
* 副本管理
* Copyright (C) ddl
* 2018
*/
#pragma once

struct idmgr;
class CInstanceMgr
{
public:
	CInstanceMgr();
	~CInstanceMgr();

	bool Init();
	void Run();
	void Destroy();

	// 传入副本基本ID，返回副本实例ID
	int AddInstance(int instancebaseid);

private:

	void CheckAndRemove();
	void ReleaseInstanceAndID(CInstance *scene);

private:

	std::list<CInstance *> m_InstanceList;
	std::list<CInstance *> m_WaitRemove;
	std::vector<CInstance *> m_InstanceSet;
	idmgr *m_IDPool;
};