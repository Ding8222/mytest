/*
*
*	��ͼ����
*
*/

/*
	���ж����ĵ�ͼ��Ϣ
*/

#pragma once
#include "string"

class mapinfo
{
public:
	mapinfo();
	~mapinfo();

	bool init(std::string bar_filename);
	bool load();
	void addbar(int row,int col); //����赲��

private:
	int m_mapid;
	int m_width;
	int m_height;

	bool *m_barinfo; //�赲��Ϣ
	std::string m_bar_filename;
};