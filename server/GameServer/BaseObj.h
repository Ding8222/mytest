#pragma once
#include <unordered_map>
#include "GlobalDefine.h"
#include "platform_config.h"

extern int player_delay_time;
extern int64 g_currenttime;

class CScene;
enum eObjPos
{
	EOP_X = 0,
	EOP_Y = 1,
	EOP_Z = 2,
	EOP_MAX,
};

enum eObjType
{
	EOT_PLAYER = 0,
	EOT_MONSTER = 1,
	EOT_NPC = 2,
	EOT_MAX ,
};

class CBaseObj
{
public:
	CBaseObj(int8 nObjType);
	~CBaseObj();

	virtual void Run();

	// 移动到某个坐标
	bool MoveTo(float x, float y, float z);

	// 获取当前坐标
	void GetNowPos(float &x, float &y, float &z);
	float *GetNowPos() { return m_NowPos; }

	// 设置当前坐标
	void SetNowPos(const float &x, const float &y, const float &z);

	// 添加对象至AoiList
	void AddToAoiList(CBaseObj * p);

	// 从AoiList中移除对象
	void DelFromAoiList(uint32 id);

	// 添加对象至AoiList
	void AddToAoiListOut(CBaseObj * p);

	// 从AoiList中移除对象
	void DelFromAoiListOut(uint32 id);

	// 离开AOI
	void LeaveAoi();

	// 计算Obj距离,处理AoiList
	void AoiRun();

	// 获取视野中的对象
	std::unordered_map<uint32, CBaseObj *> *GetAoiList() { return &m_AoiList; };
	
	// 获取临时ID
	uint32 GetTempID() { return m_TempID; }

	// 设置临时ID
	void SetTempID(uint32 id) { m_TempID = id; }

	// 设置当前场景
	void SetScene(CScene *_Scene);

	// 获取当前场景
	CScene* GetScene() { return m_Scene; }

	// 离开当前场景
	bool LeaveScene();
private:
	// 当前地图ID
	int m_NowMapID;
	// 当前坐标
	float m_NowPos[EOP_MAX];
	//临时ID
	uint32 m_TempID;
	// 所在场景
	CScene *m_Scene;
	// 视野中的对象
	std::unordered_map<uint32, CBaseObj *> m_AoiList;
	// 视野外的对象
	std::unordered_map<uint32, CBaseObj *> m_AoiListOut;

public:
	int8 GetObjType() { return m_ObjType; }
	bool IsPlayer() { return m_ObjType == EOT_PLAYER; }
	bool IsMonster() { return m_ObjType == EOT_MONSTER; }
	bool IsNPC() { return m_ObjType == EOT_NPC; }
	char *GetName() { return m_ObjName; }
	void SetName(const char *_Name) {
		strncpy_s(m_ObjName, _Name, MAX_NAME_LEN); m_ObjName
			[MAX_NAME_LEN - 1] = '\0';
	}
	const char *GetAoiMode() { return m_AoiMode; }
private:
	// 对象类型
	int8 m_ObjType;
	// 名称
	char m_ObjName[MAX_NAME_LEN];
	// Aoi模式
	char m_AoiMode[MAX_AOIMODE_LEN];
public:
	void SetWaitRemove() { m_WaitRemoveTime = g_currenttime; }
	bool IsWaitRemove() { return m_WaitRemoveTime > 0; }
	bool CanRemove(int64 time) { return time >= m_WaitRemoveTime + player_delay_time; }

private:
	// 待移除时间
	int64 m_WaitRemoveTime;
};