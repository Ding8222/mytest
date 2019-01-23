#include "GameServer.h"
#include "GameGatewayMgr.h"
#include "GameCenterConnect.h"
#include "PlayerMgr.h"
#include "SceneMgr.h"
#include "GameLevelMgr.h"
#include "InstanceMgr.h"
#include "MapConfig.h"
#include "Timer.h"
#include "ServerLog.h"
#include "LogConnecter.h"
#include "CSVLoad.h"
#include "BackCommand.h"
#include "objectpool.h"
#include "LuaScript.h"

#ifdef _WIN32
#include <windows.h>
#include "CtrlHandler.h"

#define delaytime(v)	Sleep(v)
#else
#include <unistd.h>

#define delaytime(v)	usleep((v) * 1000)
#define system(a)
#endif

#include "Config.h"

int64 g_currenttime;

CGameServer::CGameServer()
{
	m_BackCommand = nullptr;
	m_Run = false;
}

CGameServer::~CGameServer()
{
	m_BackCommand = nullptr;
	m_Run = false;
}

static void cb()
{
	GameServer.Destroy();
}

bool CGameServer::Init()
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
		if (!LuaScript.Init())
		{
			RunStateError("初始化 LuaScript 失败!");
			break;
		}

		if (!LuaScript.DoInitScriptFile("Script/Init.lua"))
		{
			RunStateError("初始化 LuaScript 失败!");
			break;
		}

		if (!InitBackCommand())
		{
			RunStateError("初始化 BackCommand 失败!");
			break;
		}

		if (!GameGatewayMgr.Init(
			Config.GetServerIP(),
			Config.GetServerID(),
			Config.GetListenPort(),
			Config.GetOverTime()))
		{
			RunStateError("初始化GameGatewayMgr失败!");
			break;
		}

		if (!GameCenterConnect.Init())
		{
			RunStateError("初始化中心服务器连接失败!");
			break;
		}

		if (!LogConnecter.Init(
			Config.GetLogServerIP(),
			Config.GetLogServerPort(),
			Config.GetLogServerID(),
			Config.GetLogServerName(),
			Config.GetServerID(),
			Config.GetServerType(),
			Config.GetServerName(),
			Config.GetPingTime(),
			Config.GetOverTime()))
		{
			RunStateError("初始化 LogConnecter 失败!");
			break;
		}


		if (!CSVData::Init())
		{
			RunStateError("加载CSVData失败!");
			break;
		}

		// 需要在场景初始化之前初始化MapConfig
		if (!MapConfig.Init())
		{
			RunStateError("初始化MapConfig失败!");
			break;
		}

		if (!GameLevelMgr.Init())
		{
			RunStateError("初始化GameLevelMgr失败!");
			break;
		}

		if (!PlayerMgr.init())
		{
			RunStateError("初始化PlayerMgr失败!");
			break;
		}

		if (!SceneMgr.Init())
		{
			RunStateError("初始化Scenemgr失败!");
			break;
		}

		if (!InstanceMgr.Init())
		{
			RunStateError("初始化InstanceMgr失败!");
			break;
		}

		RunStateLog("[%d线]逻辑服务器启动成功!", Config.GetLineID());
		m_Run = true;
		return true;
	} while (true);

	GameGatewayMgr.Destroy();
	GameCenterConnect.Destroy();
	LogConnecter.Destroy();
	MapConfig.Destroy();
	PlayerMgr.Destroy();
	SceneMgr.Destroy();
	GameLevelMgr.Destroy();
	InstanceMgr.Destroy();
	CSVData::Destroy();
	LuaScript.Destroy();
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
		GameGatewayMgr.ResetMsgNum();
		GameCenterConnect.ResetMsgNum();
		LogConnecter.ResetMsgNum();
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
			ElapsedLog("%s", GameGatewayMgr.GetMsgNumInfo());
			ElapsedLog("中心服务器连接：%s", GameCenterConnect.GetMsgNumInfo());
			ElapsedLog("日志服务器连接：%s", LogConnecter.GetMsgNumInfo());
		}
	}
	delaytime(300);

	PlayerMgr.Destroy();
	InstanceMgr.Destroy();
	SceneMgr.Destroy();
	GameLevelMgr.Destroy();
	MapConfig.Destroy();
	GameGatewayMgr.Destroy();
	GameCenterConnect.Destroy();
	LogConnecter.Destroy();
	CSVData::Destroy();

	Destroy();
}

void CGameServer::Exit()
{
	m_Run = false;
}

void CGameServer::RunOnce()
{
	lxnet::net_run();
	m_BackCommand->Run(g_currenttime);

	GameGatewayMgr.Run();
	GameCenterConnect.Run();
	LogConnecter.Run();

	PlayerMgr.Run();
	SceneMgr.Run();
	InstanceMgr.Run();

	GameGatewayMgr.EndRun();
	GameCenterConnect.EndRun();
	LogConnecter.EndRun();
}

void CGameServer::Destroy()
{
	Exit();

	if (m_BackCommand)
	{
		m_BackCommand->~CBackCommand();
		free(m_BackCommand);
		m_BackCommand = NULL;
	}
}

static void ProcessCommand(lxnet::Socketer *sock, const char *commandstr);

static void back_dofunction(lxnet::Socketer *sock)
{
	MessagePack *pack;
	char commandstr[32];
	Msg *pmsg = sock->GetMsg();
	if (!pmsg)
		return;
	pack = (MessagePack *)pmsg;
	pack->Begin();
	pack->GetString(commandstr, sizeof(commandstr));
	commandstr[sizeof(commandstr) - 1] = 0;

	ProcessCommand(sock, commandstr);
}

bool CGameServer::InitBackCommand()
{
	m_BackCommand = (CBackCommand *)malloc(sizeof(CBackCommand));
	if (!m_BackCommand)
		return false;
	new(m_BackCommand) CBackCommand();
	if (!m_BackCommand->Init(back_dofunction, Config.GetMonitorPort(), Config.GetPingTime(), Config.GetServerName()))
		return false;

	return true;
}

static void ProcessCommand(lxnet::Socketer *sock, const char *commandstr)
{
	static char s_buf[32 * 1024];
	short size = 0;
	MessagePack res;
	if (strcmp(commandstr, "help") == 0)
	{
		snprintf(s_buf, sizeof(s_buf) - 1, "help 帮助\nopenelapsed/closeelapsed 打开/关闭帧开销实时日志\ncurrentinfo 输出当前信息\nnetmeminfo 输出网络库内存使用情况\nallmeminfo 输出此程序内存池使用信息到文件\n");
		size = static_cast<short>(strlen(s_buf)) + 1;
		s_buf[size] = 0;
		res.PushString(s_buf);
		sock->SendMsg(&res);
	}
	else if (strcmp(commandstr, "openelapsed") == 0)
	{
		g_elapsed_log_flag = true;

		snprintf(s_buf, sizeof(s_buf) - 1, "帧开销实时日志已打开");
		size = static_cast<short>(strlen(s_buf)) + 1;
		s_buf[size] = 0;
		res.PushString(s_buf);
		sock->SendMsg(&res);
	}
	else if (strcmp(commandstr, "closeelapsed") == 0)
	{
		g_elapsed_log_flag = false;

		snprintf(s_buf, sizeof(s_buf) - 1, "帧开销实时日志已关闭");
		size = static_cast<short>(strlen(s_buf)) + 1;
		s_buf[size] = 0;
		res.PushString(s_buf);
		sock->SendMsg(&res);
	}
	else if (strcmp(commandstr, "currentinfo") == 0)
	{
		size = 0;

		GameCenterConnect.GetCurrentInfo(&s_buf[size], sizeof(s_buf) - size - 1);
		size = static_cast<short>(strlen(s_buf));
		GameGatewayMgr.GetCurrentInfo(&s_buf[size], sizeof(s_buf) - size - 1);
		size = static_cast<short>(strlen(s_buf));
		PlayerMgr.GetCurrentInfo(&s_buf[size], sizeof(s_buf) - size - 1);
		size = static_cast<short>(strlen(s_buf));
		SceneMgr.GetCurrentInfo(&s_buf[size], sizeof(s_buf) - size - 1);
		size = static_cast<short>(strlen(s_buf));
		InstanceMgr.GetCurrentInfo(&s_buf[size], sizeof(s_buf) - size - 1);
		s_buf[sizeof(s_buf) - 1] = 0;
		res.PushString(s_buf);
		sock->SendMsg(&res);
	}
	else if (strcmp(commandstr, "netmeminfo") == 0)
	{
		snprintf(s_buf, sizeof(s_buf) - 1, "%s", lxnet::net_get_memory_info(s_buf, sizeof(s_buf) - 1));
		size = static_cast<short>(strlen(s_buf)) + 1;
		s_buf[size] = 0;
		res.PushString(s_buf);
		sock->SendMsg(&res);
	}
	else if (strcmp(commandstr, "allmeminfo") == 0)
	{
		sPoolInfo.writeinfotofile();

		snprintf(s_buf, sizeof(s_buf) - 1, "所有内存信息已经写入到文件");
		size = static_cast<short>(strlen(s_buf)) + 1;
		s_buf[size] = 0;
		res.PushString(s_buf);
		sock->SendMsg(&res);
	}
	else
	{
		res.PushString(commandstr);
		sock->SendMsg(&res);
	}
}