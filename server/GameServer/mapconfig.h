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
	const std::list<CMapInfo*>& GetMapList();
	int GetMapSetSize() { return static_cast<int>(m_MapSet.size()); }
	bool isValidMapID(int32 id);
private:
	std::list<CMapInfo*> m_MapList;
	std::vector<CMapInfo*> m_MapSet;
	std::set<int32> m_TotalMapList;
};