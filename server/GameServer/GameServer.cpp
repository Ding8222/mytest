#include "stdfx.h"
#include "GameServer.h"
#include "GameGatewayMgr.h"
#include "GameCenterConnect.h"
#include "scenemgr.h"
#include "mapconfig.h"
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
			log_error("初始化CtrlHandler失败!");
			return false;
		}
#endif

		if (!CGameGatewayMgr::Instance().Init(
			CConfig::Instance().GetServerID(),
			CConfig::Instance().GetListenPort(),
			CConfig::Instance().GetOverTime()))
		{
			log_error("初始化GameGatewayMgr失败!");
			return false;
		}

		if (!CGameCenterConnect::Instance().Init())
		{
			log_error("初始化中心服务器连接失败!");
			return 0;
		}

		if (!CMapConfig::Instance().Init())
		{
			log_error("初始化MapConfig失败!");
			return false;
		}

		if (!CScenemgr::Instance().Init())
		{
			log_error("初始化Scenemgr失败!");
			return false;
		}

		m_Run = true;
		return true;
	} while (true);

	CGameGatewayMgr::Instance().Destroy();
	CGameCenterConnect::Instance().Destroy();
	CMapConfig::Instance().Destroy();
	CScenemgr::Instance().Destroy();
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

		g_currenttime = get_millisecond();
		RunOnce();
		delay = (int)(get_millisecond() - g_currenttime);
		if (delay < normaldelay)
		{
			delaytime(normaldelay - delay);
		}
		else if (delay > maxdelay)
		{
			log_error("运行超时:%d\n收到消息数量：%d，发送消息数量：%d\n%s", delay, 
				CGameCenterConnect::Instance().GetRecvMsgNum(),
				CGameCenterConnect::Instance().GetSendMsgNum(),
				CGameGatewayMgr::Instance().GetMsgNumInfo());
		}
	}
	delaytime(300);

	CGameGatewayMgr::Instance().Destroy();
	CGameCenterConnect::Instance().Destroy();
	CMapConfig::Instance().Destroy();
	CScenemgr::Instance().Destroy();

	Destroy();
}

void CGameServer::Exit()
{
	m_Run = false;
}

void CGameServer::RunOnce()
{
		lxnet::net_run();
		CScenemgr::Instance().Run();
		CGameGatewayMgr::Instance().Run();
		CGameCenterConnect::Instance().Run();

		CGameGatewayMgr::Instance().EndRun();
		CGameCenterConnect::Instance().EndRun();
}

void CGameServer::Destroy()
{
	Exit();
}
