/*
��ȡ��ͼ����
*/
#pragma once
#include<unordered_map>

class CMapInfo;

class CMapConfig
{
public:
	CMapConfig();
	~CMapConfig();

	static CMapConfig &Instance()
	{
		static CMapConfig m;
		return m;
	}

	// ��ʼ����ȡ���е�ͼ����
	bool Init();
	// ���ص�ͼ�е��赲��
	bool LoadBar(CMapInfo* map);
	// ��ȡ��ͼ��Ϣָ��
	const CMapInfo *GetMapInfo(int mapid);
	// ��ȡ���е�ͼ
	const std::unordered_map<int, CMapInfo*>& GetMapList();
private:
	//���е�Map��Ϣ
	std::unordered_map<int, CMapInfo*> m_MapList;
};