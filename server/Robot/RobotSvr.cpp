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
	CRobotSvr::Instance().Destroy();
}

bool CRobotSvr::Init()
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
		if (!CRobotMgr::Instance().Init(CConfig::Instance().GetLoginServerIP().c_str(),
			CConfig::Instance().GetLoginServerPort(),
			CConfig::Instance().GetLoginServerID(),
			CConfig::Instance().GetMaxRobot(),
			CConfig::Instance().GetPingTime(),
			CConfig::Instance().GetOverTime()))
		{
			RunStateError("初始化client mgr 失败!");
			break;
		}
		
		m_Run = true;
		return true;
	} while (true);

	CRobotMgr::Instance().Destroy();
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
// 			log_error("运行超时:%d\n收到消息数量：%d，发送消息数量：%d\n", delay,
// 				CRobotMgr::Instance().GetRecvMsgNum(),
// 				CRobotMgr::Instance().GetSendMsgNum());
		}
	}
	delaytime(300);

	CRobotMgr::Instance().Destroy();

	Destroy();
}

void CRobotSvr::Exit()
{
	m_Run = false;
}

void CRobotSvr::RunOnce()
{
	lxnet::net_run();
	CRobotMgr::Instance().Run();

	CRobotMgr::Instance().EndRun();
}

void CRobotSvr::Destroy()
{
	Exit();
}
