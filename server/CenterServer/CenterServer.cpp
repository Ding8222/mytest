#include "CenterServer.h"
#include "CentServerMgr.h"
#include "config.h"
#include "Timer.h"
#include "ServerLog.h"
#include "lxnet\base\crosslib.h"
#include "ServerStatusMgr.h"
#include "ClientAuthMgr.h"
#include "LogConnecter.h"
#include "CenterPlayerMgr.h"
#include "NameCheckConnecter.h"
#include "BackCommand.h"
#include "objectpool.h"
#include "TeamMgr.h"

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

CCenterServer::CCenterServer()
{
	m_BackCommand = nullptr;
	m_Run = false;
}

CCenterServer::~CCenterServer()
{
	m_BackCommand = nullptr;
	m_Run = false;
}

static void cb()
{
	CenterServer.Destroy();
}

bool CCenterServer::Init()
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

		if (!TeamMgr.Init())
		{
			RunStateError("初始化 TeamMgr 失败!");
			break;
		}

		if (!CentServerMgr.Init(
			Config.GetServerIP(),
			Config.GetServerID(),
			Config.GetListenPort(),
			Config.GetOverTime()))
		{
			RunStateError("初始化 ServerMgr 失败!");
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

		if (!NameCheckConnecter.Init(
			Config.GetNameCheckServerIP(),
			Config.GetNameCheckServerPort(),
			Config.GetNameCheckServerID(),
			Config.GetNameCheckServerName(),
			Config.GetServerID(),
			Config.GetServerType(),
			Config.GetServerName(),
			Config.GetPingTime(),
			Config.GetOverTime()))
		{
			RunStateError("初始化 NameCheckConnecter 失败!");
			break;
		}

		m_Run = true;
		return true;

	} while (true);

	CentServerMgr.Destroy();
	LogConnecter.Destroy();
	NameCheckConnecter.Destroy();
	ClientAuthMgr.Destroy();
	ServerStatusMgr.Destroy();
	CenterPlayerMgr.Destroy();
	Destroy();

	return false;
}

bool CCenterServer::Release()
{
	return true;
}

void CCenterServer::Run()
{
	const int normaldelay = 50;
	const int maxdelay = normaldelay * 2;
	int delay;
	while (m_Run)
	{
		CentServerMgr.ResetMsgNum();
		LogConnecter.ResetMsgNum();
		NameCheckConnecter.ResetMsgNum();
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
			ElapsedLog("运行超时:%d\n%s日志服务器连接：%s名称检查服务器连接：%s", delay, 
				CentServerMgr.GetMsgNumInfo(), 
				LogConnecter.GetMsgNumInfo(),
				NameCheckConnecter.GetMsgNumInfo());
		}
	}
	delaytime(300);

	CentServerMgr.Destroy();
	LogConnecter.Destroy();
	NameCheckConnecter.Destroy();
	ClientAuthMgr.Destroy();
	ServerStatusMgr.Destroy();
	CenterPlayerMgr.Destroy();

	Destroy();
}

void CCenterServer::Exit()
{
	m_Run = false;
}

void CCenterServer::RunOnce()
{
	lxnet::net_run();
	m_BackCommand->Run(g_currenttime);

	CentServerMgr.Run();
	LogConnecter.Run();
	NameCheckConnecter.Run();

	CentServerMgr.EndRun();
	LogConnecter.EndRun();
	NameCheckConnecter.EndRun();
}

void CCenterServer::Destroy()
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

bool CCenterServer::InitBackCommand()
{
	m_BackCommand = (CBackCommand *)malloc(sizeof(CBackCommand));
	if (!m_BackCommand)
		return false;
	new(m_BackCommand) CBackCommand();
	if (!m_BackCommand->Init(back_dofunction,Config.GetMonitorPort(),Config.GetPingTime(), Config.GetServerName()))
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

		CentServerMgr.GetCurrentInfo(&s_buf[size], sizeof(s_buf) - size - 1);
		size = static_cast<short>(strlen(s_buf));
		ClientAuthMgr.GetCurrentInfo(&s_buf[size], sizeof(s_buf) - size - 1);
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