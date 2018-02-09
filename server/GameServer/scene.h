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
	int count;
	int max;
	int current;
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
	CBaseObj * GetObj(int id);
	// 返回是否可以移动到某个点
	bool bCanMove(int x, int y, int z);
	// 移动至某个点
	bool MoveTo(CBaseObj * obj, float x, float y, float z);
	// Aoi Run
	void Message();
	// 更新对象在Aoi中的位置
	void Update(uint32 id, const char * mode, float pos[3]);
	// 获取当前场景所属的MapID
	int GetMapID() { return m_MapID; }

public:
	// 副本相关
	void SetInsranceID(int id) { m_InstanceID = id; }
	int GetInsranceID() { return m_InstanceID; }
	void SetRemoveTime(int64 currenttime) { m_RemoveTime = currenttime; }
	bool IsNeedRemove() { return m_RemoveTime > 0; }
	bool CanRemove(int64 currenttime) { if (!IsNeedRemove()) return false; return currenttime >= m_RemoveTime; }
private:
	//副本ID
	int m_InstanceID;
	//待移除时间
	int64 m_RemoveTime;
	// 所需地图ID
	int m_MapID;
	// 场景宽
	int m_Width;
	// 场景高
	int m_Height;

	// 出生点
	int m_BirthPoint_X;
	int m_BirthPoint_Y;
	int m_BirthPoint_Z;
	
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