#include "LogServer.h"
#include "LogServerMgr.h"
#include "config.h"
#include "Timer.h"
#include "ServerLog.h"
#include "crosslib.h"

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

CLogServer::CLogServer()
{
	m_Run = false;
}

CLogServer::~CLogServer()
{
	m_Run = false;
}

static void cb()
{
	CLogServer::Instance().Destroy();
}

bool CLogServer::Init()
{
	do
	{
#ifdef _WIN32
		if (!CCtrlHandler::Instance().Init(&cb))
		{
			RunStateError("初始化CtrlHandler失败!");
			return false;
		}
#endif
		if (!CLogServerMgr::Instance().Init())
		{
			RunStateError("初始化 LogServerMgr 失败!");
			break;
		}

		m_Run = true;
		return true;

	} while (true);

	CLogServerMgr::Instance().Destroy();
	Destroy();

	return false;
}

bool CLogServer::Release()
{
	return true;
}

void CLogServer::Run()
{
	const int normaldelay = 50;
	const int maxdelay = normaldelay * 2;
	int delay;
	while (m_Run)
	{
		CLogServerMgr::Instance().ResetMsgNum();
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
			ElapsedLog("运行超时:%d\n%s", delay, CLogServerMgr::Instance().GetMsgNumInfo());
		}
	}
	delaytime(300);

	CLogServerMgr::Instance().Destroy();

	Destroy();
}

void CLogServer::Exit()
{
	m_Run = false;
}

void CLogServer::RunOnce()
{
	lxnet::net_run();

	CLogServerMgr::Instance().Run();

	CLogServerMgr::Instance().EndRun();
}

void CLogServer::Destroy()
{
	Exit();
}