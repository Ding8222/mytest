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
	CScene *FindScene(int32 mapid);
	bool AddNPC(int32 npcid, int32 mapid, float x, float y, float z);
	bool AddMonster(int32 monsterid, int32 mapid, float x, float y, float z);
private:
	// 添加游戏场景
	bool AddScene(CMapInfo* mapconfig);
	// 地图初始化的时候，创建所有NPC
	bool LoadNPC();
	// 地图初始化的时候，创建所有Monster
	bool LoadMonster();
private:
	// mapid,scene
	std::unordered_map<int32 ,CScene *> m_SceneMap;
};