/*
* 机器人
* Copyright (C) ddl
* 2018
*/
#pragma once

class CRobotSvr
{
public:
	CRobotSvr();
	~CRobotSvr();

	static CRobotSvr &Instance()
	{
		static CRobotSvr m;
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