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

	// ��ʼ��
	bool Init(CMapInfo * _mapinfo, aoi_space * space, alloc_cookie * cookie);
	// Run ÿ֡��
	void Run();

	// ���볡��
	bool AddObj(CBaseObj * obj);
	// ��ȡ�����еĶ���
	CBaseObj * GetObj(uint32_t id);
	// �����Ƿ�����ƶ���ĳ����
	bool bCanMove(int x, int y, int z);
	// �ƶ���ĳ����
	bool MoveTo(CBaseObj * obj, float x, float y, float z);
	// Aoi Run
	void Message();
	// ���¶�����Aoi�е�λ��
	void Update(uint32_t id, const char * mode, float pos[3]);
	// ����һ����ʱID
	inline uint32_t GetTempID() { return ++m_TempID; };
	// ��ȡ��ǰ����������MapID
	inline int GetMapID() { return m_MapID; }
private:
	// �����ͼID
	int m_MapID;
	// ������
	int m_Width;
	// ������
	int m_Height;

	// ������
	int m_BirthPoint_X;
	int m_BirthPoint_Y;
	int m_BirthPoint_Z;

	// ������ʱID
	uint32_t m_TempID;

	// �赲��Ϣ
	bool *m_Barinfo;
	// ��ͼ��Ϣ
	CMapInfo *m_MapInfo;
	// Aoi�ڴ������Ϣ
	struct alloc_cookie* m_Cookie;
	// Aoi �ռ�
	struct aoi_space * m_Space;
	// �Ƿ���¹�Aoi
	bool m_bMessage;
	// �����ж���map
	std::unordered_map<uint32_t, CBaseObj *> m_ObjMap;
};