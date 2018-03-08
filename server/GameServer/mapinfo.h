/*
* 单个地图数据
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "string"

class CMapInfo
{
public:
	CMapInfo();
	~CMapInfo();

	bool Init(int mapid, const char *bar_filename);
	void Destroy();

	void GetMapBirthPoint(int &x, int &y, int &z);
	int  GetMapID();
	void GetMapWidthAndHeight(int &x, int &y);
	bool *GetBarInfo() { return m_BarInfo; }
private:
	int m_Mapid;
	int m_Width;
	int m_Height;

	int m_BirthPoint_X;
	int m_BirthPoint_Y;
	int m_BirthPoint_Z;

	//阻挡信息
	bool *m_BarInfo;
};