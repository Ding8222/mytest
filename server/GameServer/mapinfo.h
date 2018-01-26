/*
*
*	地图数据
*
*/

/*
	处理表中读到的地图信息
*/

#pragma once
#include "string"

class CMapInfo
{
public:
	CMapInfo();
	~CMapInfo();

	bool Init(int mapid, std::string bar_filename);
	// 获取出生点
	void GetMapBirthPoint(int &x, int &y, int &z);
	// 获取地图id
	int  GetMapID();
	// 获取地图宽高
	void GetMapWidthAndHeight(int &x, int &y);
	// 获取地图阻挡点文件名称
	inline const std::string &GetBarFileName() { return s_BarFileName; }
	// 设置地图阻挡点信息
	void SetMapBarInfo(int width, int height, bool* barinfo) {
		m_Width = width; m_Height = height; m_BarInfo = barinfo;
	};
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
	std::string s_BarFileName;
};