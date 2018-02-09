/*
* 场景管理
* Copyright (C) ddl
* 2018
*/
#pragma once
#include<unordered_map>

struct idmgr;
class CScene;
class CMapInfo;

class CScenemgr
{
public:
	CScenemgr();
	~CScenemgr();

	static CScenemgr &Instance()
	{
		static CScenemgr m;
		return m;
	}

	bool Init();
	void Destroy();
	void Run();

	void CheckAndRemove();
	int AddInstance(int instancebaseid);
	CScene *FindScene(int mapid);
private:
	bool AddScene(CMapInfo* mapconfig);
	void ReleaseInstanceAndID(CScene *scene);
private:

	// mapid,scene
	std::unordered_map<int ,CScene *> m_SceneMap;
	// 副本
	std::list<CScene *> m_InstanceList;
	std::list<CScene *> m_WaitRemove;
	std::vector<CScene *> m_InstanceSet;
	idmgr *m_IDPool;
};