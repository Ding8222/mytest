#include "RobotSvr.h"
#include "RobotMgr.h"
#include "Config.h"
#include "serverlog.h"
#include "crosslib.h"
#include "lxnet.h"
#include "Timer.h"

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

CRobotSvr::CRobotSvr()
{
	m_Run = false;
}

CRobotSvr::~CRobotSvr()
{
	m_Run = false;
}

static void cb()
{
	RobotSvr.Destroy();
}

bool CRobotSvr::Init(int offset)
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
		if (!RobotMgr.Init(Config.GetLoginServerIP().c_str(),
			Config.GetLoginServerPort(),
			Config.GetLoginServerID(),
			Config.GetMaxRobot(),
			Config.GetPingTime(),
			Config.GetOverTime(), offset))
		{
			RunStateError("初始化client mgr 失败!");
			break;
		}
		
		m_Run = true;
		return true;
	} while (true);

	RobotMgr.Destroy();
	Destroy();

	return false;
}

bool CRobotSvr::Release()
{
	return true;
}

void CRobotSvr::Run()
{
	const int normaldelay = 50;
	const int maxdelay = normaldelay * 2;
	int delay;
	while (m_Run)
	{
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
			ElapsedLog("运行超时:%d\n", delay);
		}
	}
	delaytime(300);

	RobotMgr.Destroy();

	Destroy();
}

void CRobotSvr::Exit()
{
	m_Run = false;
}

void CRobotSvr::RunOnce()
{
	lxnet::net_run();
	RobotMgr.Run();

	RobotMgr.EndRun();
}

void CRobotSvr::Destroy()
{
	Exit();
}
