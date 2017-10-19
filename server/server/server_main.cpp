/*
*
*	Server的main函数
*
*/

#include"stdfx.h"

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
		scenemgr::Instance().run();
		//clientmgr run 逻辑相关的都在这里面跑
		clientmgr::Instance().run();
		//clientmgr endrun  主要是将前面加到队列中的msg在这边实际的发送出去
		clientmgr::Instance().endrun();
		//检测当前帧使用时间，未达到最小值的时候，sleep
		delay = static_cast<int>(get_millisecond() - g_currenttime);
		if (delay < 100)
			delaytime(100 - delay);
		else
			std::cout << "run time out :" << delay << std::endl;
	}
}

bool init()
{
	if (!mapconfig::Instance().init())
	{
		std::cout << "init map failed!" << std::endl;
		return false;
	}

	if (!scenemgr::Instance().init())
	{
		std::cout << "init map failed!" << std::endl;
		return false;
	}

	return true;
}

int main(void)
{
	std::cout << "server start!" << std::endl;

	//读取配置文件
	if (!config::Instance().init())
	{
		std::cout << "init config error!" << std::endl;
		system("pause");
		return 0;
	}
	//初始化网络库
	if (!lxnet::net_init(512, 1, 1024 * 32, 100, 1, 4, 1))
	{
		std::cout << "init network error!" << std::endl;
		system("pause");
		return 0;
	}
	//设置监听端口，创建listener
	if (!clientmgr::Instance().init(config::Instance().GetListenPort(), config::Instance().GetClientOverTime()))
	{
		std::cout << "init clientmgr error!" << std::endl;
		system("pause");
		return 0;
	}

	if (!init())
	{
		std::cout << "server init failed!" << std::endl;
		system("pause");
		return 0;
	}
	
	g_run = true;
	//死循环
	run();
	//循环结束后的资源释放
	clientmgr::Instance().release();
	delaytime(1000);

	lxnet::net_release();
	system("pause");
	return 0;
}