/*
* 数据服务器
* Copyright (C) ddl
* 2018
*/
#pragma once

class CBackCommand;
class CDBServer
{
public:
	CDBServer();
	~CDBServer();

	static CDBServer &Instance()
	{
		static CDBServer m;
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