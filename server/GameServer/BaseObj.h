#pragma once
#include<string>
#include<unordered_map>

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
	bool MoveTo(float &x, float &y, float &z);
	// 获取当前坐标
	void GetNowPos(float &x, float &y, float &z);
	// 设置当前坐标
	void SetNowPos(const float &x, const float &y, const float &z);
	// 添加对象至AoiList
	void AddToAoiList(CBaseObj * p);
	// 从AoiList中移除对象
	void DelFromAoiList(uint32_t id);
	// 添加对象至AoiList
	void AddToAoiListOut(CBaseObj * p);
	// 从AoiList中移除对象
	void DelFromAoiListOut(uint32_t id);
	// AoiList清理
	void AoiRun();
	// 获取名称
	std::string GetName() { return m_ObjName; }
	// 设置名称
	void SetName(std::string _Name) { m_ObjName = _Name; }
	// 获取临时ID
	uint32_t GetTempID() { return m_TempID; }
	// 设置临时ID
	void SetTempID(uint32_t id) { m_TempID = id; }
	// 设置当前场景
	void SetScene(CScene *_Scene);
	//获取当前场景
	CScene* GetScene() { return m_Scene; }
private:

	// 当前地图ID
	int m_NowMapID;

	// 当前坐标
	float m_NowPos[EPP_MAX];

	// 当前临时ID
	int m_TempID;

	CScene *m_Scene;
	std::string m_ObjName;
	std::unordered_map<uint32_t, CBaseObj *> m_AoiList;
	// 视野外的对象
	std::unordered_map<uint32_t, CBaseObj *> m_AoiListOut;
};