#include "config.h"
#include "GameGateway.h"
#include "GameConnect.h"
#include "GateClientMgr.h"
#include "GateCenterConnect.h"
#include "Timer.h"
#include "ServerLog.h"
#include "ClientAuth.h"
#include "LogConnecter.h"

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

CGameGateway::CGameGateway()
{
	m_Run = false;
}

CGameGateway::~CGameGateway()
{
	m_Run = false;
}

static void cb()
{
	CGameGateway::Instance().Destroy();
}

bool CGameGateway::Init()
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

		if (!CGateClientMgr::Instance().Init(CConfig::Instance().GetMaxClientNum(),
			CConfig::Instance().GetListenPort(),
			CConfig::Instance().GetOverTime(),
			CConfig::Instance().GetRecvDataLimt(),
			CConfig::Instance().GetSendDataLimt()))
		{
			RunStateError("初始化GateClientMgr失败!");
			break;
		}

		if (!CGateCenterConnect::Instance().Init())
		{
			RunStateError("初始化中心服务器连接失败!");
			break;
		}

		if (!CGameConnect::Instance().Init())
		{
			RunStateError("初始化逻辑服务器连接失败!");
			break;
		}

		if (!CLogConnecter::Instance().Init(
			CConfig::Instance().GetLogServerIP(),
			CConfig::Instance().GetLogServerPort(),
			CConfig::Instance().GetLogServerID(),
			CConfig::Instance().GetLogServerName(),
			CConfig::Instance().GetServerID(),
			CConfig::Instance().GetServerType(),
			CConfig::Instance().GetServerName(),
			CConfig::Instance().GetPingTime(),
			CConfig::Instance().GetOverTime()))
		{
			RunStateError("初始化 LogConnecter 失败!");
			break;
		}

		m_Run = true;
		return true;
	} while (true);

	CGateClientMgr::Instance().Destroy();
	CGateCenterConnect::Instance().Destroy();
	CGameConnect::Instance().Destroy();
	CLogConnecter::Instance().Destroy();
	CClientAuth::Instance().Destroy();
	Destroy();

	return false;
}

bool CGameGateway::Release()
{
	return true;
}

void CGameGateway::Run()
{
	const int normaldelay = 50;
	const int maxdelay = normaldelay * 2;
	int delay;
	while (m_Run)
	{
		CGateCenterConnect::Instance().ResetMsgNum();
		CGameConnect::Instance().ResetMsgNum();
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
			ElapsedLog("逻辑服务器连接：%s", CGameConnect::Instance().GetMsgNumInfo());
			ElapsedLog("中心服务器连接：%s", CGateCenterConnect::Instance().GetMsgNumInfo());
			ElapsedLog("日志服务器连接：%s", CLogConnecter::Instance().GetMsgNumInfo());
		}
	}
	delaytime(300);

	CGateClientMgr::Instance().Destroy();
	CGateCenterConnect::Instance().Destroy();
	CGameConnect::Instance().Destroy();
	CLogConnecter::Instance().Destroy();
	CClientAuth::Instance().Destroy();

	Destroy();
}

void CGameGateway::Exit()
{
	m_Run = false;
}

void CGameGateway::RunOnce()
{
	lxnet::net_run();

	CGameConnect::Instance().Run();
	CGateClientMgr::Instance().Run();
	CGateCenterConnect::Instance().Run();
	CLogConnecter::Instance().Run();

	CGameConnect::Instance().EndRun();
	CGateClientMgr::Instance().EndRun();
	CGateCenterConnect::Instance().EndRun();
	CLogConnecter::Instance().EndRun();
}

void CGameGateway::Destroy()
{
	Exit();
}