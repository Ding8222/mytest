/*
* 中心服务器
* Copyright (C) ddl
* 2018
*/

#include "stdfx.h"
#include "config.h"
#include "centerserver.h"
#include "NetConfig.h"

#ifdef _WIN32
#include <windows.h>
#include "MiniDump.h"

#define delaytime(v)	Sleep(v)
#else
#include <unistd.h>

#define delaytime(v)	usleep(v * 1000)
#define system(a)
#endif

bool init()
{
	log_error("中心服务器开始启动!");

#ifdef _WIN32
	if (!CMiniDump::Begin())
	{
		log_error("初始化MiniDump失败!");
		system("pause");
		return false;
	}
#endif

	//读取网络配置文件
	if (!CNetConfig::Instance().Init())
	{
		log_error("初始化NetConfig失败!");
		system("pause");
		return 0;
	}

	//读取服务器配置文件
	if (!CConfig::Instance().Init("CenterServer"))
	{
		log_error("初始化CenterServer Config失败!");
		system("pause");
		return 0;
	}
	//初始化网络库
	if (!lxnet::net_init(CNetConfig::Instance().GetBigBufSize(), CNetConfig::Instance().GetBigBufNum(),
		CNetConfig::Instance().GetSmallBufSize(), CNetConfig::Instance().GetSmallBufNum(),
		CNetConfig::Instance().GetListenerNum(), CNetConfig::Instance().GetSocketerNum(),
		CNetConfig::Instance().GetThreadNum()))
	{
		log_error("初始化网络库失败!");
		system("pause");
		return 0;
	}
	//设置监听端口，创建listener
	if (!CCenterServer::Instance().Init())
	{
		log_error("初始化失败!");
		system("pause");
		return 0;
	}
	
	CCenterServer::Instance().Run();
	//循环结束后的资源释放
	CCenterServer::Instance().Release();
	lxnet::net_release();

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