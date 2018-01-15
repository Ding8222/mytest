/*
*
*	��ͼ����
*
*/

/*
	������ж����ĵ�ͼ��Ϣ
*/

#pragma once
#include "string"

class CMapInfo
{
public:
	CMapInfo();
	~CMapInfo();

	bool Init(int mapid, std::string bar_filename);
	// ��ȡ������
	void GetMapBirthPoint(int &x, int &y, int &z);
	// ��ȡ��ͼid
	int  GetMapID();
	// ��ȡ��ͼ���
	void GetMapWidthAndHeight(int &x, int &y);
	// ��ȡ��ͼ�赲���ļ�����
	inline const std::string &GetBarFileName() { return s_BarFileName; }
	// ���õ�ͼ�赲����Ϣ
	void SetMapBarInfo(int width, int height, bool* barinfo) {
		m_Width = width; m_Height = height; m_BarInfo = barinfo;
	};
	bool *GetBarInfo() { return m_BarInfo; }
private:
	int m_Mapid;
	int m_Width;
	int m_Height;

	int m_BirthPoint_X;
	int m_BirthPoint_Y;
	int m_BirthPoint_Z;

	//�赲��Ϣ
	bool *m_BarInfo;
	std::string s_BarFileName;
};