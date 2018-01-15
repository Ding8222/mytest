/*
*
*	Server��main����
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
		//��ȡ��ǰ֡ʱ��
		g_currenttime = get_millisecond();
		//�����run
		lxnet::net_run();
		CScenemgr::Instance().Run();
		//clientmgr run �߼���صĶ�����������
		CClientMgr::Instance().Run();
		//clientmgr endrun  ��Ҫ�ǽ�ǰ��ӵ������е�msg�����ʵ�ʵķ��ͳ�ȥ
		CClientMgr::Instance().EndRun();
		//��⵱ǰ֡ʹ��ʱ�䣬δ�ﵽ��Сֵ��ʱ��sleep
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

	//��ȡ�����ļ�
	if (!CConfig::Instance().Init())
	{
		log_error("init config error!");
		system("pause");
		return 0;
	}
	//��ʼ�������
	if (!lxnet::net_init(512, 1, 1024 * 32, 100, 1, 4, 1))
	{
		log_error("init network error!");
		system("pause");
		return 0;
	}
	//���ü����˿ڣ�����listener
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
	//��ѭ��
	run();
	//ѭ�����������Դ�ͷ�
	CClientMgr::Instance().Release();
	delaytime(1000);

	lxnet::net_release();
	system("pause");
	return 0;
}