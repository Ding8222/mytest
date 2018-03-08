/*
* 场景管理
* Copyright (C) ddl
* 2018
*/
#pragma once
#include<unordered_map>

class CScene;
class CMapInfo;

class CSceneMgr
{
public:
	CSceneMgr();
	~CSceneMgr();

	static CSceneMgr &Instance()
	{
		static CSceneMgr m;
		return m;
	}

	bool Init();
	void Destroy();
	void Run();

public:
	CScene *FindScene(int mapid);

private:
	bool AddScene(CMapInfo* mapconfig);

private:
	// mapid,scene
	std::unordered_map<int ,CScene *> m_SceneMap;
};