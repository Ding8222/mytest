/*
* 单个地图数据
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "lxnet\base\platform_config.h"

class CMapInfo
{
public:
	CMapInfo();
	~CMapInfo();

	bool Init(int32 mapid, int32 type, const char *bar_filename);
	void Destroy();

	void SetMapBirthPoint(const float &x, const float &y, const float &z);
	void GetMapBirthPoint(float &x, float &y, float &z);
	void GetMapWidthAndHeight(int32 &x, int32 &y);
	int  GetMapID() { return m_MapID; }
	bool *GetBarInfo() { return m_BarInfo; }
	int32 GetMapType() { return m_MapType; }
	bool bCanMove(int32 x, int32 y, int32 z);
private:
	int32 m_MapID;
	int32 m_MapType;
	int32 m_Width;
	int32 m_Height;

	float m_BirthPoint_X;
	float m_BirthPoint_Y;
	float m_BirthPoint_Z;

	//阻挡信息
	bool *m_BarInfo;
};