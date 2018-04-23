/*
* Log服务器
* Copyright (C) ddl
* 2018
*/
#pragma once

class CLogServer
{
public:
	CLogServer();
	~CLogServer();

	static CLogServer &Instance()
	{
		static CLogServer m;
		return m;
	}

	bool Init();
	bool Release();
	void Run();
	void Exit();

	void Destroy();
private:
	void RunOnce();
private:
	bool m_Run;
};