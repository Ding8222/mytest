#include<iostream>
#include<thread>//c++ 11���߳�
#include<string>
#include"lxnet.h"
#include "crosslib.h"
#include"msgdef.h"

#ifdef _WIN32
#include <windows.h>

#define delaytime(v)	Sleep(v)
#else
#include <unistd.h>

#define delaytime(v)	usleep(v * 1000)
#define system(a)
#endif

static bool g_run;

//ÿ5���server����һ��ping
void PingMsg(lxnet::Socketer *clientsocket)
{
	MsgPing msg;
	while (g_run)
	{
		if (!clientsocket || clientsocket->IsClose())
		{
			g_run = false;
			break;
		}
		clientsocket->SendMsg(&msg);
		delaytime(5000);
	}
	std::cout << "Ping task end!" << std::endl;
}

//�����û�����
void InputMsg(lxnet::Socketer *clientsocket)
{
	MessagePack msg;
	std::string str;
	msg.SetType(MSG_CHAT);
	while (g_run)
	{
		if (!clientsocket || clientsocket->IsClose())
		{
			g_run = false;
			break;
		}

		std::cin >> str;

		if (str == "quit")
			break;

		msg.Reset();
		msg.PushString(str.c_str());
		clientsocket->SendMsg(&msg);

		delaytime(10);
	}
	std::cout << "Input task end!" << std::endl;
}

void PrintMsg(lxnet::Socketer *clientsocket)
{
	MessagePack *recvpack = NULL;
	while (g_run)
	{
		if (!clientsocket || clientsocket->IsClose())
		{
			g_run = false;
			break;
		}

		recvpack = (MessagePack *)clientsocket->GetMsg();
		if (recvpack)
		{
			switch (recvpack->GetType())
			{
			case MSG_CHAT:
			{
				char stBuff[521] = { 0 };
				recvpack->Begin();
				recvpack->GetString(stBuff, 512);
				std::cout << "Get Msg From Server: " << stBuff << std::endl;
				break;
			}
			default:
			{

			}
			}
		}
		else
			delaytime(10);
	}
	std::cout << "Print task end!" << std::endl;
}

int main(void)
{
	//��ʼ�������
	if (!lxnet::net_init(512, 1, 1024 * 32, 100, 1, 4, 1))
	{
		std::cout << "init network error!" << std::endl;
		system("pause");
		return 0;
	}

	//����socket
	lxnet::Socketer *newclient = lxnet::Socketer::Create();

	if (!newclient)
	{
		std::cout << "create client socketer error!" << std::endl;
		system("pause");
		return 0;
	}

	//ÿ100���볢������
	while (!newclient->Connect("127.0.0.1", 30012))
	{
		delaytime(100);
	}

	std::cout << " connect succeed!" << std::endl;

	//����Ͷ�ݽ�ȡ��Ϣ
	newclient->CheckRecv();

	g_run = true;

	//���������߳�
	//����
	std::thread PingTask(PingMsg, newclient);
	//����
	std::thread InputTask(InputMsg, newclient);
	//���̷߳���
	InputTask.detach();
	//�������
	std::thread PrintTask(PrintMsg, newclient);

	//��ѭ��
	while (1)
	{
		lxnet::net_run();
		if (!newclient || newclient->IsClose())
			break;

		delaytime(100);

		newclient->CheckSend();
	}

	//�ȴ��߳̽���--����
	PingTask.join();
	PrintTask.join();

	delaytime(1000);

	lxnet::Socketer::Release(newclient);
	lxnet::net_release();
	system("pause");
	return 0;
}