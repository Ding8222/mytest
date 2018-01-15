/*
*
*	Server的main函数
*
*/

#include"stdfx.h"
#include"clientmgr.h"
#include"scenemgr.h"
#include"mapconfig.h"

#ifdef _WIN32
#include <windows.h>

#define delaytime(v)	Sleep(v)
#else
#include <unistd.h>

#define delaytime(v)	usleep(v * 1000)
#define system(a)
#endif

int64 g_currenttime;
bool g_run;
void run()
{
	int delay;
	while (g_run)
	{
		//获取当前帧时间
		g_currenttime = get_millisecond();
		//网络库run
		lxnet::net_run();
		CScenemgr::Instance().Run();
		//clientmgr run 逻辑相关的都在这里面跑
		CClientMgr::Instance().Run();
		//clientmgr endrun  主要是将前面加到队列中的msg在这边实际的发送出去
		CClientMgr::Instance().EndRun();
		//检测当前帧使用时间，未达到最小值的时候，sleep
		delay = static_cast<int>(get_millisecond() - g_currenttime);
		if (delay < 100)
			delaytime(100 - delay);
		else
			log_error("run time out : %d", delay);
	}
}

bool init()
{
	if (!CMapConfig::Instance().Init())
	{
		log_error("init mapconfig failed!");
		return false;
	}

	if (!CScenemgr::Instance().Init())
	{
		log_error("init scenemgr failed!");
		return false;
	}

	return true;
}

int main(void)
{
	log_error("server start!");

	//读取配置文件
	if (!CConfig::Instance().Init())
	{
		log_error("init config error!");
		system("pause");
		return 0;
	}
	//初始化网络库
	if (!lxnet::net_init(512, 1, 1024 * 32, 100, 1, 4, 1))
	{
		log_error("init network error!");
		system("pause");
		return 0;
	}
	//设置监听端口，创建listener
	if (!CClientMgr::Instance().Init(CConfig::Instance().GetListenPort(), CConfig::Instance().GetClientOverTime()))
	{
		log_error("init clientmgr error!");
		system("pause");
		return 0;
	}

	if (!init())
	{
		log_error("server init failed!");
		system("pause");
		return 0;
	}
	
	g_run = true;
	//死循环
	run();
	//循环结束后的资源释放
	CClientMgr::Instance().Release();
	delaytime(1000);

	lxnet::net_release();
	system("pause");
	return 0;
}