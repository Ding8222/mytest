/*
* 场景数据
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>
extern "C"
{
#include "aoi.h"
}
#include "platform_config.h"

class CMapInfo;
class CBaseObj;
struct idmgr;

struct alloc_cookie {
	alloc_cookie()
	{
		count = 0;
		max = 0;
		current = 0;
	}
	~alloc_cookie()
	{
		count = 0;
		max = 0;
		current = 0;
	}
	int32 count;
	int32 max;
	int32 current;
};

class CScene
{
public:
	CScene();
	~CScene();

	bool Init(CMapInfo * _mapinfo);
	void Destroy();
	void Run();

	// 进入场景
	bool AddObj(CBaseObj * obj);
	// 退出场景
	bool DelObj(CBaseObj * obj);
	// 获取场景中的对象
	CBaseObj * GetObj(int32 id);
	// 返回是否可以移动到某个点
	bool bCanMove(int32 x, int32 y, int32 z);
	// 移动至某个点
	bool MoveTo(CBaseObj * obj, float x, float y, float z);
	// Aoi Run
	void Message();
	// 更新对象在Aoi中的位置
	void Update(uint32 id, const char * mode, float pos[3]);
	// 获取当前场景所属的MapID
	int32 GetMapID() { return m_MapID; }

private:
	// 所需地图ID
	int32 m_MapID;
	// 场景宽
	int32 m_Width;
	// 场景高
	int32 m_Height;

	// 出生点
	float m_BirthPoint_X;
	float m_BirthPoint_Y;
	float m_BirthPoint_Z;
	
	// 阻挡信息
	bool *m_Barinfo;
	// Aoi内存分配信息
	struct alloc_cookie* m_Cookie;
	// Aoi 空间
	struct aoi_space * m_Space;
	// 是否更新过Aoi
	bool m_bMessage;

	std::unordered_map<uint32, CBaseObj *> m_ObjMap;

	std::vector<CBaseObj *> m_ObjSet;
	idmgr *m_IDPool;
};