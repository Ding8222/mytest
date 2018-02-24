#include "stdfx.h"
#include "DBServer.h"
#include "DBCenterConnect.h"
#include "config.h"
#include "Timer.h"
#include "ServerLog.h"

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

CDBServer::CDBServer()
{
	m_Run = false;
}

CDBServer::~CDBServer()
{
	m_Run = false;
}

static void cb()
{
	CDBServer::Instance().Destroy();
}

bool CDBServer::Init()
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

		if (!CDBCenterConnect::Instance().Init())
		{
			log_error("初始化 ServerMgr 失败!");
			break;
		}

		m_Run = true;
		return true;

	} while (true);

	CDBCenterConnect::Instance().Destroy();
	Destroy();

	return false;
}

bool CDBServer::Release()
{
	return true;
}

void CDBServer::Run()
{
	const int normaldelay = 50;
	const int maxdelay = normaldelay * 2;
	int delay;
	while (m_Run)
	{
		CDBCenterConnect::Instance().ResetMsgNum();
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
			ElapsedLog("运行超时:%d\n%s", delay, CDBCenterConnect::Instance().GetMsgNumInfo());
		}
	}
	delaytime(300);

	CDBCenterConnect::Instance().Destroy();

	Destroy();
}

void CDBServer::Exit()
{
	m_Run = false;
}

void CDBServer::RunOnce()
{
	lxnet::net_run();

	CDBCenterConnect::Instance().Run();

	CDBCenterConnect::Instance().EndRun();
}

void CDBServer::Destroy()
{
	Exit();
}