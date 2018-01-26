/*
读取地图配置
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

	// 初始化读取所有地图配置
	bool Init();
	void Destroy();
	// 加载地图中的阻挡点
	bool LoadBar(CMapInfo* map);
	// 获取地图信息指针
	const CMapInfo *GetMapInfo(int mapid);
	// 获取所有地图
	const std::unordered_map<int, CMapInfo*>& GetMapList();
private:
	//所有的Map信息
	std::unordered_map<int, CMapInfo*> m_MapList;
};