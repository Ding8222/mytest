/*
* 机器人服务器
* Copyright (C) ddl
* 2018
*/

#include "vld.h"
#include "RobotSvr.h"
#include "config.h"
#include "NetConfig.h"
#include "ServerLog.h"
#include "lxnet.h"
#include "objectpool.h"
#include "google/protobuf/message.h"

#ifdef _WIN32
#include <windows.h>
#include "MiniDump.h"
#include <locale.h>

#define delaytime(v)	Sleep(v)
#else
#include <unistd.h>

#define delaytime(v)	usleep((v) * 1000)
#define system(a)
#endif

bool init()
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

	if (!init_log("Robot_Log"))
	{
		RunStateError("初始化Log失败!");
		return false;
	}

	// 读取网络配置文件
	if (!NetConfig.Init())
	{
		RunStateError("初始化NetConfig失败!");
		system("pause");
		return 0;
	}

	// 读取配置文件
	if (!Config.Init("Robot"))
	{
		RunStateError("初始化Config失败!");
		system("pause");
		return 0;
	}

	sPoolInfo.SetMeminfoFileName("log_log/Robot_Log/mempoolinfo.txt");
	log_writelog("机器人开始启动!");

	// 初始化网络库
	if (!lxnet::net_init(NetConfig.GetBigBufSize(), NetConfig.GetBigBufNum(),
		NetConfig.GetSmallBufSize(), NetConfig.GetSmallBufNum(),
		NetConfig.GetListenerNum(), NetConfig.GetSocketerNum(),
		NetConfig.GetThreadNum()))
	{
		RunStateError("初始化网络库失败!");
		system("pause");
		return 0;
	}
	// 初始化
	if (!RobotSvr.Init())
	{
		RunStateError("初始化LoginServer失败!");
		system("pause");
		return 0;
	}

	RobotSvr.Run();
	// 循环结束后的资源释放
	RobotSvr.Release();
	lxnet::net_release();
	release_log();
	google::protobuf::ShutdownProtobufLibrary();

#ifdef _WIN32
	CMiniDump::End();
#endif
	return true;
}

int main(void)
{
	init();
	return 0;
}