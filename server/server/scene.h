#pragma once
#include <unordered_map>
extern "C"
{
#include "aoi.h"
}

class CMapInfo;
class CBaseObj;

struct alloc_cookie {
	int count;
	int max;
	int current;
};

class CScene
{
public:
	CScene();
	~CScene();

	// 初始化
	bool Init(CMapInfo * _mapinfo, aoi_space * space, alloc_cookie * cookie);
	// Run 每帧跑
	void Run();

	// 进入场景
	bool AddObj(CBaseObj * obj);
	// 获取场景中的对象
	CBaseObj * GetObj(uint32_t id);
	// 返回是否可以移动到某个点
	bool bCanMove(int x, int y, int z);
	// 移动至某个点
	bool MoveTo(CBaseObj * obj, float x, float y, float z);
	// Aoi Run
	void Message();
	// 更新对象在Aoi中的位置
	void Update(uint32_t id, const char * mode, float pos[3]);
	// 生成一个临时ID
	inline uint32_t GetTempID() { return ++m_TempID; };
	// 获取当前场景所属的MapID
	inline int GetMapID() { return m_MapID; }
private:
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

	// 场景临时ID
	uint32_t m_TempID;

	// 阻挡信息
	bool *m_Barinfo;
	// 地图信息
	CMapInfo *m_MapInfo;
	// Aoi内存分配信息
	struct alloc_cookie* m_Cookie;
	// Aoi 空间
	struct aoi_space * m_Space;
	// 是否更新过Aoi
	bool m_bMessage;
	// 场景中对象map
	std::unordered_map<uint32_t, CBaseObj *> m_ObjMap;
};