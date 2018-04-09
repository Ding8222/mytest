/*
* 单个地图数据
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <string>
#include "platform_config.h"

class CMapInfo
{
public:
	CMapInfo();
	~CMapInfo();

	bool Init(int32 mapid, int8 type, const char *bar_filename);
	void Destroy();

	void SetMapBirthPoint(float &x, float &y, float &z);
	void GetMapBirthPoint(float &x, float &y, float &z);
	int  GetMapID();
	void GetMapWidthAndHeight(int32 &x, int32 &y);
	bool *GetBarInfo() { return m_BarInfo; }
	int8 GetMapType() { return m_MapType; }
private:
	int32 m_MapID;
	int8 m_MapType;
	int32 m_Width;
	int32 m_Height;

	float m_BirthPoint_X;
	float m_BirthPoint_Y;
	float m_BirthPoint_Z;

	//阻挡信息
	bool *m_BarInfo;
};