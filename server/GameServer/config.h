#pragma once
#include <string>
#include"BaseConfig.h"

#define Config CConfig::Instance()
class CConfig :public CBaseConfig
{
public:
	CConfig();
	~CConfig();

	static CConfig &Instance()
	{
		static CConfig m;
		return m;
	}

	bool Init(const char *servername = "", int lineid = 0);

	void SetLineID(int id) { m_LineID = id; }
	int GetLineID() { return m_LineID; }
	int GetMaxClientNum() { return m_MaxClientNum; }

private:
	int m_LineID;
	int m_MaxClientNum;
};