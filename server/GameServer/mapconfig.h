/*
* 地图配置管理
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "platform_config.h"
#include <set>
#include <vector>
#include <list>

class CMapInfo;

#define MapConfig CMapConfig::Instance()

class CMapConfig
{
public:
	CMapConfig();
	~CMapConfig();

	static CMapConfig &Instance()
	{
		static CMapConfig m;
		return m;
	}

	bool Init();
	void Destroy();

	CMapInfo *FindMapInfo(int32 mapid);
	// 获取本服所有地图信息
	const std::list<CMapInfo*>& GetMapList();
	// 获取本服加载的地图个数
	int GetMapSetSize() { return static_cast<int>(m_MapSet.size()); }
	// 判断地图ID是否有效
	bool isValidMapID(int32 id);
private:
	// 本服加载的所有地图信息
	std::list<CMapInfo*> m_MapList;
	std::vector<CMapInfo*> m_MapSet;
	// 所有地图id，包括不在本服加载的地图id
	std::set<int32> m_TotalMapList;
};