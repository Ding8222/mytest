/*
* 网关服务器
* Copyright (C) ddl
* 2018
*/
#pragma once

class CGameGateway
{
public:
	CGameGateway();
	~CGameGateway();

	static CGameGateway &Instance()
	{
		static CGameGateway m;
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