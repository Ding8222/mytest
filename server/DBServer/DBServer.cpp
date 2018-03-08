#include "DBServer.h"
#include "DBCenterConnect.h"
#include "ClientLogin.h"
#include "config.h"
#include "Timer.h"
#include "ServerLog.h"
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
			RunStateError("初始化CtrlHandler失败!");
			break;
		}
#endif

		if (!CDBCenterConnect::Instance().Init())
		{
			RunStateError("初始化 ServerMgr 失败!");
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

		m_Run = true;
		return true;

	} while (true);

	CDBCenterConnect::Instance().Destroy();
	CLogConnecter::Instance().Destroy();
	CClientLogin::Instance().Destroy();
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
			ElapsedLog("中心服务器连接：%s", CDBCenterConnect::Instance().GetMsgNumInfo());
			ElapsedLog("日志服务器连接：%s", CLogConnecter::Instance().GetMsgNumInfo());
		}
	}
	delaytime(300);

	CDBCenterConnect::Instance().Destroy();
	CLogConnecter::Instance().Destroy();
	CClientLogin::Instance().Destroy();

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
	CLogConnecter::Instance().Run();

	CDBCenterConnect::Instance().EndRun();
	CLogConnecter::Instance().EndRun();
}

void CDBServer::Destroy()
{
	Exit();
}