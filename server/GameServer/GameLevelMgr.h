/*
* 关卡管理
* Copyright (C) ddl
* 2019
*/
#pragma once
#include<unordered_map>
#include "GameLevel.h"

#define GameLevelMgr CGameLevelMgr::Instance()

class CGameLevel;

class CGameLevelMgr
{
public:
	CGameLevelMgr();
	~CGameLevelMgr();

	static CGameLevelMgr &Instance()
	{
		static CGameLevelMgr m;
		return m;
	}

	bool Init();
	void Destroy();

public:
	bool InitGameLevel(int32 biglevelid, std::vector<int32> &midlevellist, std::vector<int32> &sublevellist, std::vector<stMapData *> &gamelevelinfo);
	bool GenGameLevel(int32 biglevelid, int32 midlevelid, int32 sublevelid, std::vector<stMapData *> &gamelevelinfo);
private:
	int32 mapgrid[CGameLevel::m_MapGriden][CGameLevel::m_MapGriden];
	// 单个关卡集合
	std::vector<CGameLevel *> m_GameLevelList;
};