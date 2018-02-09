/*
* 地图配置管理
* Copyright (C) ddl
* 2018
*/
#pragma once
#include<unordered_map>

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

	CMapInfo *FindMapInfo(int mapid);
	const std::list<CMapInfo*>& GetMapList();
	int GetMapSetSize() { return static_cast<int>(m_MapSet.size()); }
private:
	std::list<CMapInfo*> m_MapList;
	std::vector<CMapInfo*> m_MapSet;
};