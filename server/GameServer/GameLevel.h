/*
* 关卡布局数据
* Copyright (C) ddl
* 2019
*/
#pragma once
#include <unordered_map>

enum eGatePos
{
	up = 1,
	down = 2,
	left = 3,
	right = 4,
};

// 传送门信息
struct stGateData
{
	// 传送门位置
	eGatePos nGatePos;
	// 目标地图索引
	int32 nTargetMapIndex;
	// 目标地图传送点坐标
	eGatePos nTargetMapPos;
};

// 坐标点
struct stPoint
{
	stPoint()
	{
		nX = 0;
		nY = 0;
	}

	stPoint(int32 x, int32 y)
	{
		nX = x;
		nY = y;
	}

	int32 nX;
	int32 nY;
};

// 地图信息
struct stMapData
{
	stMapData()
	{
		nMapIndex = 0;
		nMapPosX = 0;
		nMapPosY = 0;
		nMapID = 0;
		mGataInfo = nullptr;
	}

	int32 nMapIndex;
	int32 nMapPosX;
	int32 nMapPosY;
	int32 nMapID;
	std::vector<stGateData *> *mGataInfo;
};

class CGameLevel
{
public:
	// 关卡长度
	static const int32 m_GameLevelLen = 6;
	// 用于生成关卡的格子大小
	static const int32 m_MapGriden = 2 * m_GameLevelLen - 1;

	CGameLevel();
	~CGameLevel();

	bool Init(int32 mapgrid[][m_MapGriden]);
	void Destroy();

	static std::vector<stPoint> m_Point;
private:
	// 获取新的地图所在坐标
	stPoint GetPos(int32 mapgrid[][m_MapGriden]);
	// 生成传送门
	void GenGate(int32 mapgrid[][m_MapGriden], int32 x, int32 y);
	// 生成地图
	void GenMap(int32 mapgrid[][m_MapGriden], int32 n, int32 x, int32 y);

public:
	// 传送门信息
	std::unordered_map<int32, std::vector<stGateData *> *> m_GateInfo;
	// 地图坐标信息
	std::unordered_map<int32, stPoint *> m_MapPosInfo;
};