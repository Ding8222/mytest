/*
* 对象场景
* Copyright (C) ddl
* 2018
*/

#pragma once
#include <unordered_map>
#include "lxnet\base\platform_config.h"
#include "Vector3D.h"
#include "AoiDefine.h"

class CScene;
class CBaseObj;
enum eObjPos
{
	EOP_X = 0,
	EOP_Y = 1,
	EOP_Z = 2,
	EOP_MAX,
};

class CObjScene
{
public:
	CObjScene();
	~CObjScene();

	virtual CBaseObj *GetObj() = 0;

	// 移动到某个坐标
	bool MoveTo(float x, float y, float z);

	// 设置当前坐标
	void SetNowPos(const Vector3D &pos);
	void SetNowPos(const float &x, const float &y, const float &z);
	void GetNowPos(float &x, float &y, float &z);
	float *GetNowPos() { return m_NowPos; }
	Vector3D &GetNowPosV() { return m_vNowPos; }
	float GetNowPosX() { return m_NowPos[EOP_X]; }
	float GetNowPosY() { return m_NowPos[EOP_Y]; }
	float GetNowPosZ() { return m_NowPos[EOP_Z]; }
	// 获取朝向
	void SetHeading(const Vector3D &heading) { m_vHeading = heading; }
	Vector3D &GetHeading() { return m_vHeading; }

	// 设置出生点坐标
	void SetHomePos(const float &x, const float &y, const float &z);
	Vector3D &GetHomePos() { return m_vHomePos; }

	// 添加对象至AoiList
	void AddToAoiList(CBaseObj * p);
	// 从AoiList中移除对象
	void DelFromAoiList(uint32 id);
	// 添加对象至AoiList
	void AddToAoiListOut(CBaseObj * p);
	// 从AoiList中移除对象
	void DelFromAoiListOut(uint32 id);
	// 从视野中查找目标
	CBaseObj * FindFromAoiList(uint32 id);
	// 获取视野中的对象
	std::unordered_map<uint32, CBaseObj *> *GetAoiList() { return &m_AoiList; };
	// 离开AOI
	void LeaveAoi();

	// 计算Obj距离,处理AoiList
	void AoiRun();

	// 设置当前所在地图ID
	void SetMapID(int32 id) { m_NowMapID = id; }
	int32 GetMapID() { return m_NowMapID; }

	// 设置临时ID
	void SetTempID(uint32 id) { m_TempID = id; }
	uint32 GetTempID() { return m_TempID; }

	// 设置当前场景
	void SetScene(CScene *_Scene);
	CScene* GetScene() { return m_Scene; }
	bool LeaveScene();

	// 设置AOI模式
	void SetAoiMode(const char *mode) { if (mode)strncpy_s(m_AoiMode, mode, MAX_AOIMODE_LEN - 1); }
	const char *GetAoiMode() { return m_AoiMode; }
private:
	// 当前地图ID
	int m_NowMapID;
	// 当前坐标
	float m_NowPos[EOP_MAX];
	Vector3D m_vNowPos;
	// 朝向
	Vector3D m_vHeading;
	// 出生点
	Vector3D m_vHomePos;
	//临时ID
	uint32 m_TempID;
	// 所在场景
	CScene *m_Scene;
	// 视野中的对象
	std::unordered_map<uint32, CBaseObj *> m_AoiList;
	// 视野外的对象
	std::unordered_map<uint32, CBaseObj *> m_AoiListOut;
	// Aoi模式
	char m_AoiMode[MAX_AOIMODE_LEN];
};
