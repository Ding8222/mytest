﻿/*
* 机器人
* Copyright (C) ddl
* 2018
*/
#pragma once

#define RobotSvr CRobotSvr::Instance()
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

	bool Init(int offset);
	bool Release();
	void Run();
	void Exit();
	void Destroy();

private:
	void RunOnce();

private:
	bool m_Run;
};