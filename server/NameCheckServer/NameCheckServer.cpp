#include "NameCheckServer.h"
#include "NameCheckServerMgr.h"
#include "config.h"
#include "Timer.h"
#include "ServerLog.h"
#include "lxnet\base\crosslib.h"
#include "NameSet.h"

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

CCheckNameServer::CCheckNameServer()
{
	m_Run = false;
}

CCheckNameServer::~CCheckNameServer()
{
	m_Run = false;
}

static void cb()
{
	CheckNameServer.Destroy();
}

bool CCheckNameServer::Init()
{
	do
	{
#ifdef _WIN32
		if (!CtrlHandler.Init(&cb))
		{
			RunStateError("初始化CtrlHandler失败!");
			break;
		}
#endif
		if (!NameCheckServerMgr.Init(
			Config.GetServerIP(),
			Config.GetServerID(),
			Config.GetListenPort(),
			Config.GetOverTime()))
		{
			RunStateError("初始化 NameCheckServerMgr 失败!");
			break;
		}

		m_Run = true;
		return true;

	} while (true);

	NameCheckServerMgr.Destroy();
	NameSet.Destroy();

	Destroy();

	return false;
}

bool CCheckNameServer::Release()
{
	return true;
}

void CCheckNameServer::Run()
{
	const int normaldelay = 50;
	const int maxdelay = normaldelay * 2;
	int delay;
	while (m_Run)
	{
		NameCheckServerMgr.ResetMsgNum();
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
			ElapsedLog("运行超时:%d\n%s", delay, NameCheckServerMgr.GetMsgNumInfo());
		}
	}
	delaytime(300);

	NameCheckServerMgr.Destroy();
	NameSet.Destroy();

	Destroy();
}

void CCheckNameServer::Exit()
{
	m_Run = false;
}

void CCheckNameServer::RunOnce()
{
	lxnet::net_run();

	NameCheckServerMgr.Run();

	NameCheckServerMgr.EndRun();
}

void CCheckNameServer::Destroy()
{
	Exit();
}