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

	// �ƶ���ĳ������
	bool MoveTo(float &x, float &y, float &z);
	// ��ȡ��ǰ����
	void GetNowPos(float &x, float &y, float &z);
	// ���õ�ǰ����
	void SetNowPos(const float &x, const float &y, const float &z);
	// ��Ӷ�����AoiList
	void AddToAoiList(CBaseObj * p);
	// ��AoiList���Ƴ�����
	void DelFromAoiList(uint32_t id);
	// ��Ӷ�����AoiList
	void AddToAoiListOut(CBaseObj * p);
	// ��AoiList���Ƴ�����
	void DelFromAoiListOut(uint32_t id);
	// AoiList����
	void AoiRun();
	// ��ȡ����
	std::string GetName() { return m_ObjName; }
	// ��������
	void SetName(std::string _Name) { m_ObjName = _Name; }
	// ��ȡ��ʱID
	uint32_t GetTempID() { return m_TempID; }
	// ������ʱID
	void SetTempID(uint32_t id) { m_TempID = id; }
	// ���õ�ǰ����
	void SetScene(CScene *_Scene);
	//��ȡ��ǰ����
	CScene* GetScene() { return m_Scene; }
private:

	// ��ǰ��ͼID
	int m_NowMapID;

	// ��ǰ����
	float m_NowPos[EPP_MAX];

	// ��ǰ��ʱID
	int m_TempID;

	CScene *m_Scene;
	std::string m_ObjName;
	std::unordered_map<uint32_t, CBaseObj *> m_AoiList;
	// ��Ұ��Ķ���
	std::unordered_map<uint32_t, CBaseObj *> m_AoiListOut;
};