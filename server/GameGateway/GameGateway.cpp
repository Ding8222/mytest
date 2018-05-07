#include "config.h"
#include "GameGateway.h"
#include "GameConnect.h"
#include "GateClientMgr.h"
#include "GateCenterConnect.h"
#include "Timer.h"
#include "ServerLog.h"
#include "ClientAuth.h"
#include "LogConnecter.h"
#include "BackCommand.h"
#include "objectpool.h"

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
	m_BackCommand = nullptr;
	m_Run = false;
}

CGameGateway::~CGameGateway()
{
	m_BackCommand = nullptr;
	m_Run = false;
}

static void cb()
{
	GameGateway.Destroy();
}

bool CGameGateway::Init()
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
		if (!InitBackCommand())
		{
			RunStateError("初始化 BackCommand 失败!");
			break;
		}

		if (!GateClientMgr.Init(Config.GetMaxClientNum(),
			Config.GetListenPort(),
			Config.GetOverTime(),
			Config.GetRecvDataLimt(),
			Config.GetSendDataLimt()))
		{
			RunStateError("初始化GateClientMgr失败!");
			break;
		}

		if (!GateCenterConnect.Init())
		{
			RunStateError("初始化中心服务器连接失败!");
			break;
		}

		if (!GameConnect.Init())
		{
			RunStateError("初始化逻辑服务器连接失败!");
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

		m_Run = true;
		return true;
	} while (true);

	GateClientMgr.Destroy();
	GateCenterConnect.Destroy();
	GameConnect.Destroy();
	LogConnecter.Destroy();
	ClientAuth.Destroy();
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
		GateCenterConnect.ResetMsgNum();
		GameConnect.ResetMsgNum();
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
			ElapsedLog("运行超时:%d,%I64d", delay, g_currenttime);
			ElapsedLog("逻辑服务器连接：%s", GameConnect.GetMsgNumInfo());
			ElapsedLog("中心服务器连接：%s", GateCenterConnect.GetMsgNumInfo());
			ElapsedLog("日志服务器连接：%s", LogConnecter.GetMsgNumInfo());
		}
	}
	delaytime(300);

	GateClientMgr.Destroy();
	GateCenterConnect.Destroy();
	GameConnect.Destroy();
	LogConnecter.Destroy();
	ClientAuth.Destroy();

	Destroy();
}

void CGameGateway::Exit()
{
	m_Run = false;
}

void CGameGateway::RunOnce()
{
	lxnet::net_run();
	m_BackCommand->Run(g_currenttime);

	GameConnect.Run();
	GateClientMgr.Run();
	GateCenterConnect.Run();
	LogConnecter.Run();

	GameConnect.EndRun();
	GateClientMgr.EndRun();
	GateCenterConnect.EndRun();
	LogConnecter.EndRun();
}

void CGameGateway::Destroy()
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

bool CGameGateway::InitBackCommand()
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
		size = (short)strlen(s_buf) + 1;
		s_buf[size] = 0;
		res.PushString(s_buf);
		sock->SendMsg(&res);
	}
	else if (strcmp(commandstr, "openelapsed") == 0)
	{
		g_elapsed_log_flag = true;

		snprintf(s_buf, sizeof(s_buf) - 1, "帧开销实时日志已打开");
		size = (short)strlen(s_buf) + 1;
		s_buf[size] = 0;
		res.PushString(s_buf);
		sock->SendMsg(&res);
	}
	else if (strcmp(commandstr, "closeelapsed") == 0)
	{
		g_elapsed_log_flag = false;

		snprintf(s_buf, sizeof(s_buf) - 1, "帧开销实时日志已关闭");
		size = (short)strlen(s_buf) + 1;
		s_buf[size] = 0;
		res.PushString(s_buf);
		sock->SendMsg(&res);
	}
	else if (strcmp(commandstr, "currentinfo") == 0)
	{
		size = 0;

		GateCenterConnect.GetCurrentInfo(&s_buf[size], sizeof(s_buf) - size - 1);
		size = strlen(s_buf);
		GameConnect.GetCurrentInfo(&s_buf[size], sizeof(s_buf) - size - 1);
		size = strlen(s_buf);
		GateClientMgr.GetCurrentInfo(&s_buf[size], sizeof(s_buf) - size - 1);
		s_buf[sizeof(s_buf) - 1] = 0;
		res.PushString(s_buf);
		sock->SendMsg(&res);
	}
	else if (strcmp(commandstr, "netmeminfo") == 0)
	{
		snprintf(s_buf, sizeof(s_buf) - 1, "%s", lxnet::net_get_memory_info(s_buf, sizeof(s_buf) - 1));
		size = (short)strlen(s_buf) + 1;
		s_buf[size] = 0;
		res.PushString(s_buf);
		sock->SendMsg(&res);
	}
	else if (strcmp(commandstr, "allmeminfo") == 0)
	{
		sPoolInfo.writeinfotofile();

		snprintf(s_buf, sizeof(s_buf) - 1, "所有内存信息已经写入到文件");
		size = (short)strlen(s_buf) + 1;
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