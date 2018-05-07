/*
* 中心服务器
* Copyright (C) ddl
* 2018
*/
#pragma once

class CBackCommand;
#define CenterServer CCenterServer::Instance()
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
	bool InitBackCommand();
private:
	bool m_Run;
	CBackCommand *m_BackCommand;
};