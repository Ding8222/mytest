#include "crosslib.h"
#include "loginsvr.h"
#include "LoginClientMgr.h"
#include "LoginCenterConnect.h"
#include "config.h"
#include "Timer.h"
#include "ServerLog.h"
#include "ClientAuth.h"

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

CLoginServer::CLoginServer()
{
	m_Run = false;
}

CLoginServer::~CLoginServer()
{
	m_Run = false;
}

static void cb()
{
	CLoginServer::Instance().Destroy();
}

bool CLoginServer::Init()
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
		if (!CLoginClientMgr::Instance().Init(CConfig::Instance().GetMaxClientNum(),
			CConfig::Instance().GetListenPort(),
			CConfig::Instance().GetOverTime(),
			CConfig::Instance().GetRecvDataLimt(),
			CConfig::Instance().GetSendDataLimt()))
		{
			RunStateError("初始化client mgr 失败!");
			return 0;
		}

		if (!CLoginCenterConnect::Instance().Init())
		{
			RunStateError("初始化中心服务器连接失败!");
			return 0;
		}

		m_Run = true;
		return true;
	} while (true);

	CLoginClientMgr::Instance().Destroy();
	CLoginCenterConnect::Instance().Destroy();
	CClientAuth::Instance().Destroy();
	Destroy();

	return false;
}

bool CLoginServer::Release()
{
	return true;
}

void CLoginServer::Run()
{
	const int normaldelay = 50;
	const int maxdelay = normaldelay * 2;
	int delay;
	while (m_Run)
	{
		CLoginCenterConnect::Instance().ResetMsgNum();
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
			ElapsedLog("运行超时:%d\n%s", delay, CLoginCenterConnect::Instance().GetMsgNumInfo());
		}
	}
	delaytime(300);

	CLoginCenterConnect::Instance().Destroy();
	CLoginClientMgr::Instance().Destroy();
	CClientAuth::Instance().Destroy();

	Destroy();
}

void CLoginServer::Exit()
{
	m_Run = false;
}

void CLoginServer::RunOnce()
{
	lxnet::net_run();
	CLoginCenterConnect::Instance().Run();
	CLoginClientMgr::Instance().Run();

	CLoginCenterConnect::Instance().EndRun();
	CLoginClientMgr::Instance().EndRun();
}

void CLoginServer::Destroy()
{
	Exit();
}
