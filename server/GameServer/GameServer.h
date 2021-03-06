﻿/*
* 逻辑服务器
* Copyright (C) ddl
* 2018
*/
#pragma once

class CBackCommand;
#define GameServer CGameServer::Instance()
class CGameServer
{
public:
	CGameServer();
	~CGameServer();

	static CGameServer &Instance()
	{
		static CGameServer m;
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