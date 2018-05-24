/*
* 逻辑服务器
* Copyright (C) ddl
* 2018
*/
#include "vld.h"
#include "config.h"
#include "GameServer.h"
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

void init(int argc, char *argv[])
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

		if (argc < 2)
		{
			RunStateError("没有填写线路ID！");
			break;
		}

		int nLineID = atoi(argv[1]);
		if (nLineID <= 0)
		{
			RunStateError("线路ID填写错误！");
			break;
		}
		if (!init_log(fmt::format("GameServer_Log_Line_{0}", nLineID).c_str()))
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
		if (!Config.Init("GameServer", nLineID))
		{
			RunStateError("初始化Config失败!");
			system("pause");
			break;
		}

		g_elapsed_log_flag = Config.IsOpenElapsedLog();
		sPoolInfo.SetMeminfoFileName("log_log/GameServer_Log/mempoolinfo.txt");

		RunStateLog("[%d线]逻辑服务器开始启动!", Config.GetLineID());

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
		//GameServer初始化
		if (!GameServer.Init())
		{
			RunStateError("初始化失败!");
			system("pause");
			break;
		}

		GameServer.Run();
		break;
	} while (true);
	RunStateLog("[%d线]逻辑服务器关闭!", Config.GetLineID());
	//循环结束后的资源释放
	GameServer.Release();
	lxnet::net_release();
	release_log();
	google::protobuf::ShutdownProtobufLibrary();
#ifdef _WIN32
	CMiniDump::End();
#endif
}

int main(int argc, char *argv[])
{
	init(argc, argv);
	return 0;
}