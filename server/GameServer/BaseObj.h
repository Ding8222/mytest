#pragma once
#include <unordered_map>
#include "GlobalDefine.h"
#include "platform_config.h"

extern int player_delay_time;
extern int64 g_currenttime;

class CScene;
enum eObjPos
{
	EPP_X = 0,
	EPP_Y = 1,
	EPP_Z = 2,
	EPP_MAX = 3,
};

class CBaseObj
{
public:
	CBaseObj();
	~CBaseObj();

	virtual void Run();

	// 移动到某个坐标
	bool MoveTo(float x, float y, float z);
	// 获取当前坐标
	void GetNowPos(float &x, float &y, float &z);
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
	// 清理AOI
	void LeaveAoi();
	// AoiList清理
	void AoiRun();
	// 获取名称
	char *GetName() { return m_ObjName; }
	// 设置名称
	void SetName(const char *_Name) {
		strncpy_s(m_ObjName, _Name, MAX_NAME_LEN); m_ObjName
			[MAX_NAME_LEN - 1] = '\0';
	}
	// 获取临时ID
	uint32 GetTempID() { return m_TempID; }
	// 设置临时ID
	void SetTempID(uint32 id) { m_TempID = id; }
	// 设置当前场景
	void SetScene(CScene *_Scene);
	//获取当前场景
	CScene* GetScene() { return m_Scene; }

public:
	void SetWaitRemove() { m_WaitRemoveTime = g_currenttime; }
	bool IsWaitRemove() { return m_WaitRemoveTime > 0; }
	bool CanRemove(int64 time) { return time >= m_WaitRemoveTime + player_delay_time; }
private:
	int64 m_WaitRemoveTime;
	// 当前地图ID
	int m_NowMapID;
	// 当前坐标
	float m_NowPos[EPP_MAX];
	// 当前临时ID
	uint32 m_TempID;

	CScene *m_Scene;
	char m_ObjName[MAX_NAME_LEN];
	std::unordered_map<uint32, CBaseObj *> m_AoiList;
	// 视野外的对象
	std::unordered_map<uint32, CBaseObj *> m_AoiListOut;
};