/*
* 中心服务器
* Copyright (C) ddl
* 2018
*/
#pragma once

class CCenterServer
{
public:
	CCenterServer();
	~CCenterServer();

	static CCenterServer &Instance()
	{
		static CCenterServer m;
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