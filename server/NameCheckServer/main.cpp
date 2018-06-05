/*
* 名字检查服务器
* Copyright (C) ddl
* 2018
*/

#include "vld.h"
#include "config.h"
#include "NameCheckServer.h"
#include "NetConfig.h"
#include "ServerLog.h"
#include "lxnet.h"
#include "google/protobuf/message.h"

#ifdef _WIN32
#include <windows.h>
#include "MiniDump.h"

#define delaytime(v)	Sleep(v)
#else
#include <unistd.h>

#define delaytime(v)	usleep((v) * 1000)
#define system(a)
#endif

void init()
{
	do
	{
#ifdef _WIN32
		SetConsoleOutputCP(65001);

		if (!CMiniDump::Begin())
		{
			RunStateError("初始化MiniDump失败!");
			system("pause");
			break;
		}
#endif

		if (!init_log("NameCheckServer_Log"))
		{
			RunStateError("初始化Log失败!");
			break;
		}

		//读取网络配置文件
		if (!NetConfig.Init())
		{
			RunStateError("初始化NetConfig失败!");
			system("pause");
			break;
		}

		//读取配置文件
		if (!Config.Init("NameCheckServer"))
		{
			RunStateError("初始化Config失败!");
			system("pause");
			break;
		}

		g_elapsed_log_flag = Config.IsOpenElapsedLog();

		RunStateLog("名称检查服务器开始启动!");

		//初始化网络库
		if (!lxnet::net_init(NetConfig.GetBigBufSize(), NetConfig.GetBigBufNum(),
			NetConfig.GetSmallBufSize(), NetConfig.GetSmallBufNum(),
			NetConfig.GetListenerNum(), NetConfig.GetSocketerNum(),
			NetConfig.GetThreadNum()))
		{
			RunStateError("初始化网络库失败!");
			system("pause");
			break;
		}
		//设置监听端口，创建listener
		if (!CheckNameServer.Init())
		{
			RunStateError("初始化失败!");
			system("pause");
			break;
		}

		CheckNameServer.Run();
		break;
	} while (true);
	RunStateLog("名称检查服务器关闭!");
	//循环结束后的资源释放
	CheckNameServer.Release();
	lxnet::net_release();
	release_log();
	google::protobuf::ShutdownProtobufLibrary();
#ifdef _WIN32
	CMiniDump::End();
#endif
}

int main(void)
{
	init();
	return 0;
}