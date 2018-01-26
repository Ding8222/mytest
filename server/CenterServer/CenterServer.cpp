﻿#include "stdfx.h"
#include "CenterServer.h"
#include "CentServerMgr.h"
#include "config.h"

#ifdef _WIN32
#include <windows.h>
#include "CtrlHandler.h"

#define delaytime(v)	Sleep(v)
#else
#include <unistd.h>

#define delaytime(v)	usleep(v * 1000)
#define system(a)
#endif

int64 g_currenttime;

CCenterServer::CCenterServer()
{
	m_Run = false;
}

CCenterServer::~CCenterServer()
{
	m_Run = false;
}

static void cb()
{
	CCenterServer::Instance().Destroy();
}

bool CCenterServer::Init()
{
	do 
	{
#ifdef _WIN32
		if (!CCtrlHandler::Instance().Init(&cb))
		{
			log_error("初始化CtrlHandler失败!");
			return false;
		}
#endif
		if (!CCentServerMgr::Instance().Init(
			CConfig::Instance().GetServerID(),
			CConfig::Instance().GetListenPort(),
			CConfig::Instance().GetOverTime()))
		{
			log_error("初始化 ServerMgr 失败!");
			break;
		}

		m_Run = true;
		return true;

	} while (true);

	CCentServerMgr::Instance().Destroy();
	Destroy();

	return false;
}

bool CCenterServer::Release()
{
	return true;
}

void CCenterServer::Run()
{
	const int normaldelay = 50;
	const int maxdelay = normaldelay * 2;
	int delay;
	while (m_Run)
	{
		CCentServerMgr::Instance().ResetMsgNum();

		g_currenttime = get_millisecond();
		RunOnce();
		delay = (int)(get_millisecond() - g_currenttime);
		if (delay < normaldelay)
		{
			delaytime(normaldelay - delay);
		}
		else if (delay > maxdelay)
		{
			log_error("运行超时:%d\n%s", delay, CCentServerMgr::Instance().GetMsgNumInfo());
		}
	}
	delaytime(300);

	CCentServerMgr::Instance().Destroy();

	Destroy();
}

void CCenterServer::Exit()
{
	m_Run = false;
}

void CCenterServer::RunOnce()
{
	lxnet::net_run();

	CCentServerMgr::Instance().Run();

	CCentServerMgr::Instance().EndRun();
}

void CCenterServer::Destroy()
{
	Exit();
}