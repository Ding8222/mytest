/*
* 玩家关卡
* Copyright (C) ddl
* 2019
*/
#pragma once
#include <unordered_map>
#include "GameLevel.h"

class CPlayerGameLevel
{
public:
	CPlayerGameLevel();
	~CPlayerGameLevel();

	bool Init();
	void Destroy();

private:
	// 根据biglevelid生成关卡
	bool GenGameLevel(int32 biglevelid);
	// 生成下一个关卡
	bool GenNextGameLevel();
private:
	// 大关卡类型
	int32 m_BigGameLevelID;
	// 大关卡索引
	int32 m_MidGameLevelIndex;
	// 大关卡顺序
	std::vector<int32> m_MidGameLevelList;
	// 小关卡索引
	int32 m_SubGameLevelIndex;
	// 小关卡非boss关顺序
	std::vector<int32> m_SubGameLevelList;
	// 当前所在地图索引
	int32 m_CurrentMapIndex;
	// 当前关卡信息
	std::vector<stMapData *> m_GameLevelInfo;
};