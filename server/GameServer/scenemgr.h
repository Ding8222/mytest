﻿/*
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

	CScene *FindScene(int mapid);
private:
	bool AddScene(CMapInfo* mapconfig);
private:

	// mapid,scene
	std::unordered_map<int ,CScene *> m_SceneMap;
};