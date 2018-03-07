/*
* 逻辑服务器
* Copyright (C) ddl
* 2018
*/

#include "config.h"
#include "GameServer.h"
#include "NetConfig.h"
#include "ServerLog.h"
#include "lxnet.h"
#include "objectpool.h"
#include "fmt/ostream.h"

#pragma comment(lib,"fmt.lib") 
#ifdef _WIN32
#include <windows.h>
#include "MiniDump.h"

#define delaytime(v)	Sleep(v)
#else
#include <unistd.h>

#define delaytime(v)	usleep((v) * 1000)
#define system(a)
#endif

bool init(int argc, char *argv[])
{
#ifdef _WIN32
	if (!CMiniDump::Begin())
	{
		RunStateError("初始化MiniDump失败!");
		system("pause");
		return false;
	}
#endif

	if (argc < 2)
	{
		RunStateError("没有填写线路ID！");
		return false;
	}

	int nLineID = atoi(argv[1]);
	if (nLineID <= 0)
	{
		RunStateError("线路ID填写错误！");
		return false;
	}
	if (!init_log(fmt::format("GameServer_Log_Line_{0}", nLineID).c_str()))
	{
		RunStateError("初始化Log失败!");
		return false;
	}
		
	//读取网络配置文件
	if (!CNetConfig::Instance().Init())
	{
		RunStateError("初始化NetConfig失败!");
		system("pause");
		return 0;
	}

	//读取配置文件
	if (!CConfig::Instance().Init("GameServer", nLineID))
	{
		RunStateError("初始化Config失败!");
		system("pause");
		return 0;
	}

	g_elapsed_log_flag = CConfig::Instance().IsOpenElapsedLog();
	sPoolInfo.SetMeminfoFileName("GameServer_Log/mempoolinfo.txt");

	RunStateLog("[%d线]逻辑服务器开始启动!", CConfig::Instance().GetLineID());

	//初始化网络库
	if (!lxnet::net_init(CNetConfig::Instance().GetBigBufSize(), CNetConfig::Instance().GetBigBufNum(),
		CNetConfig::Instance().GetSmallBufSize(), CNetConfig::Instance().GetSmallBufNum(),
		CNetConfig::Instance().GetListenerNum(), CNetConfig::Instance().GetSocketerNum(),
		CNetConfig::Instance().GetThreadNum()))
	{
		RunStateError("初始化网络库失败!");
		system("pause");
		return 0;
	}
	//GameServer初始化
	if (!CGameServer::Instance().Init())
	{
		RunStateError("初始化失败!");
		system("pause");
		return 0;
	}

	CGameServer::Instance().Run();
	//循环结束后的资源释放
	CGameServer::Instance().Release();
	lxnet::net_release();
	release_log();
#ifdef _WIN32
	CMiniDump::End();
#endif
	return true;
}

int main(int argc, char *argv[])
{
	init(argc, argv);
	return 0;
}