﻿/*
* 网关服务器
* Copyright (C) ddl
* 2018
*/

#include "vld.h"
#include "config.h"
#include "GameGateway.h"
#include "NetConfig.h"
#include "ServerLog.h"
#include "lxnet.h"
#include "objectpool.h"
#include "fmt/ostream.h"
#include "google/protobuf/message.h"

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
	SetConsoleOutputCP(65001);

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

	if (!init_log(fmt::format("GameGateway_Log_Line_{0}", nLineID).c_str()))
	{
		RunStateError("初始化Log失败!");
		return false;
	}

	//读取网络配置文件
	if (!NetConfig.Init())
	{
		RunStateError("初始化NetConfig失败!");
		system("pause");
		return 0;
	}

	//读取配置文件
	if (!Config.Init("GameGateway", nLineID))
	{
		RunStateError("初始化Config失败!");
		system("pause");
		return 0;
	}

	g_client_connectlog_flag = Config.IsOpenClientConnectLog();
	g_elapsed_log_flag = Config.IsOpenElapsedLog();
	sPoolInfo.SetMeminfoFileName("log_log/GameGateway_Log/mempoolinfo.txt");

	RunStateLog("[%d线]网关服务器开始启动!", Config.GetLineID());

	//初始化网络库
	if (!lxnet::net_init(NetConfig.GetBigBufSize(), NetConfig.GetBigBufNum(),
		NetConfig.GetSmallBufSize(), NetConfig.GetSmallBufNum(),
		NetConfig.GetListenerNum(), NetConfig.GetSocketerNum(),
		NetConfig.GetThreadNum()))
	{
		RunStateError("初始化网络库失败!");
		system("pause");
		return 0;
	}
	//设置监听端口，创建listener
	if (!GameGateway.Init())
	{
		RunStateError("初始化失败!");
		system("pause");
		return 0;
	}

	GameGateway.Run();
	RunStateLog("[%d线]网关服务器关闭!", Config.GetLineID());
	//循环结束后的资源释放
	GameGateway.Release();
	lxnet::net_release();
	release_log();
	google::protobuf::ShutdownProtobufLibrary();
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