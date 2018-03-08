﻿#include "GameServer.h"
#include "GameGatewayMgr.h"
#include "GameCenterConnect.h"
#include "PlayerMgr.h"
#include "SceneMgr.h"
#include "InstanceMgr.h"
#include "MapConfig.h"
#include "Config.h"
#include "Timer.h"
#include "ServerLog.h"
#include "LogConnecter.h"
#include "CSVLoad.h"

#ifdef _WIN32
#include <windows.h>
#include "CtrlHandler.h"

#define delaytime(v)	Sleep(v)
#else
#include <unistd.h>

#define delaytime(v)	usleep((v) * 1000)
#define system(a)
#endif

int64 g_currenttime;

CGameServer::CGameServer()
{
	m_Run = false;
}

CGameServer::~CGameServer()
{
	m_Run = false;
}

static void cb()
{
	CGameServer::Instance().Destroy();
}

bool CGameServer::Init()
{
	do 
	{
#ifdef _WIN32
		if (!CCtrlHandler::Instance().Init(&cb))
		{
			RunStateError("初始化CtrlHandler失败!");
			break;
		}
#endif

		if (!CGameGatewayMgr::Instance().Init(
			CConfig::Instance().GetServerIP(),
			CConfig::Instance().GetServerID(),
			CConfig::Instance().GetListenPort(),
			CConfig::Instance().GetOverTime()))
		{
			RunStateError("初始化GameGatewayMgr失败!");
			break;
		}

		if (!CGameCenterConnect::Instance().Init())
		{
			RunStateError("初始化中心服务器连接失败!");
			break;
		}

		if (!CLogConnecter::Instance().Init(
			CConfig::Instance().GetLogServerIP(),
			CConfig::Instance().GetLogServerPort(),
			CConfig::Instance().GetLogServerID(),
			CConfig::Instance().GetServerID(),
			CConfig::Instance().GetServerType(),
			CConfig::Instance().GetPingTime(),
			CConfig::Instance().GetOverTime()))
		{
			RunStateError("初始化 LogConnecter 失败!");
			break;
		}

		// 需要在场景初始化之前初始化MapConfig
		if (!CMapConfig::Instance().Init())
		{
			RunStateError("初始化MapConfig失败!");
			break;
		}

		if (!CSceneMgr::Instance().Init())
		{
			RunStateError("初始化Scenemgr失败!");
			break;
		}

		if (!CInstanceMgr::Instance().Init())
		{
			RunStateError("初始化InstanceMgr失败!");
			break;
		}

		if (!CSVData::Init())
		{
			RunStateError("加载CSVData失败!");
			break;
		}
		
		if (!CPlayerMgr::Instance().init())
		{
			RunStateError("初始化PlayerMgr失败!");
			break;
		}

		m_Run = true;
		return true;
	} while (true);

	CGameGatewayMgr::Instance().Destroy();
	CGameCenterConnect::Instance().Destroy();
	CLogConnecter::Instance().Destroy();
	CMapConfig::Instance().Destroy();
	CPlayerMgr::Instance().Destroy();
	CSceneMgr::Instance().Destroy();
	CInstanceMgr::Instance().Destroy();
	CSVData::Destroy();
	Destroy();

	return false;
}

bool CGameServer::Release()
{
	return true;
}

void CGameServer::Run()
{
	const int normaldelay = 100;
	const int maxdelay = normaldelay * 2;
	int delay;
	while (m_Run)
	{
		CGameGatewayMgr::Instance().ResetMsgNum();
		CGameCenterConnect::Instance().ResetMsgNum();
		CLogConnecter::Instance().ResetMsgNum();
		CTimer::UpdateTime();

		g_currenttime = get_millisecond();
		RunOnce();
		delay = (int)(get_millisecond() - g_currenttime);
		if (delay < normaldelay)
		{
			delaytime(normaldelay - delay);
		}
		else if (delay > maxdelay)
		{
			ElapsedLog("运行超时:%d", delay);
			ElapsedLog("%s", CGameGatewayMgr::Instance().GetMsgNumInfo());
			ElapsedLog("中心服务器连接：%s", CGameCenterConnect::Instance().GetMsgNumInfo());
			ElapsedLog("日志服务器连接：%s", CLogConnecter::Instance().GetMsgNumInfo());
		}
	}
	delaytime(300);

	CGameGatewayMgr::Instance().Destroy();
	CGameCenterConnect::Instance().Destroy();
	CLogConnecter::Instance().Destroy();
	CMapConfig::Instance().Destroy();
	CPlayerMgr::Instance().Destroy();
	CSceneMgr::Instance().Destroy();
	CInstanceMgr::Instance().Destroy();
	CSVData::Destroy();

	Destroy();
}

void CGameServer::Exit()
{
	m_Run = false;
}

void CGameServer::RunOnce()
{
	lxnet::net_run();
	CGameGatewayMgr::Instance().Run();
	CGameCenterConnect::Instance().Run();
	CLogConnecter::Instance().Run();

	CPlayerMgr::Instance().Run();
	CSceneMgr::Instance().Run();
	CInstanceMgr::Instance().Run();

	CGameGatewayMgr::Instance().EndRun();
	CGameCenterConnect::Instance().EndRun();
	CLogConnecter::Instance().EndRun();
}

void CGameServer::Destroy()
{
	Exit();
}
