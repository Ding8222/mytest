﻿/*
* 登陆服务器
* Copyright (C) ddl
* 2018
*/
#pragma once

class CBackCommand;
#define LoginServer CLoginServer::Instance()
class CLoginServer
{
public:
	CLoginServer();
	~CLoginServer();

	static CLoginServer &Instance()
	{
		static CLoginServer m;
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