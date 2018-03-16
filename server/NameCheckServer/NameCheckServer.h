/*
* 名称检查服务器
* Copyright (C) ddl
* 2018
*/
#pragma once

class CCheckNameServer
{
public:
	CCheckNameServer();
	~CCheckNameServer();

	static CCheckNameServer &Instance()
	{
		static CCheckNameServer m;
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