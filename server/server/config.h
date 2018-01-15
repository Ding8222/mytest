/*
*
*	server≈‰÷√
*
*/

#pragma once

class CConfig
{
public:
	CConfig();
	~CConfig();

	static CConfig &Instance()
	{
		static CConfig m;
		return m;
	}

	bool Init();
	int GetListenPort() { return m_ListenPort; }
	int GetClientOverTime() { return m_ClientOverTime; }
private:
	int m_ListenPort;
	int m_ClientOverTime;
};