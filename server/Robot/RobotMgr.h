/*
* 机器人连接管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <list>
class CRobot;
#define RobotMgr CRobotMgr::Instance()
class CRobotMgr
{
public:
	CRobotMgr();
	~CRobotMgr();

	static CRobotMgr &Instance()
	{
		static CRobotMgr m;
		return m;
	}

	bool Init(const char *ip, int port, int id, int maxrobot, int pingtime, int overtime, int offset);

	void Run();
	void EndRun();

	void Destroy();

	void ProcessRegister(CRobot *);
	void ProcessMsg(CRobot *_con);
private:
	std::string s_LoginServerIP;
	int m_LoginServerPort;
	int m_LoginServerID;

	int m_OverTime;
	int m_PingTime;
	int m_MapID;
	std::list<CRobot *> m_RobotList;
};