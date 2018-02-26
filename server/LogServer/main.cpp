/*
* Log������
* Copyright (C) ddl
* 2018
*/

#include "config.h"
#include "LogServer.h"
#include "NetConfig.h"
#include "ServerLog.h"
#include "lxnet.h"

#pragma comment(lib,"fmt.lib") 

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
#ifdef _WIN32
	if (!CMiniDump::Begin())
	{
		RunStateError("��ʼ��MiniDumpʧ��!");
		system("pause");
		return false;
	}
#endif

	if (!init_log("LogServer_Log"))
	{
		RunStateError("��ʼ��Logʧ��!");
		return false;
	}

	//��ȡ���������ļ�
	if (!CNetConfig::Instance().Init())
	{
		RunStateError("��ʼ��NetConfigʧ��!");
		system("pause");
		return 0;
	}

	//��ȡ�����ļ�
	if (!CConfig::Instance().Init("LogServer"))
	{
		RunStateError("��ʼ��Configʧ��!");
		system("pause");
		return 0;
	}

	g_elapsed_log_flag = CConfig::Instance().IsOpenElapsedLog();

	RunStateLog("��־��������ʼ����!");

	//��ʼ�������
	if (!lxnet::net_init(CNetConfig::Instance().GetBigBufSize(), CNetConfig::Instance().GetBigBufNum(),
		CNetConfig::Instance().GetSmallBufSize(), CNetConfig::Instance().GetSmallBufNum(),
		CNetConfig::Instance().GetListenerNum(), CNetConfig::Instance().GetSocketerNum(),
		CNetConfig::Instance().GetThreadNum()))
	{
		RunStateError("��ʼ�������ʧ��!");
		system("pause");
		return 0;
	}
	//���ü����˿ڣ�����listener
	if (!CLogServer::Instance().Init())
	{
		RunStateError("��ʼ��ʧ��!");
		system("pause");
		return 0;
	}

	CLogServer::Instance().Run();
	//ѭ�����������Դ�ͷ�
	CLogServer::Instance().Release();
	lxnet::net_release();
	release_log();
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