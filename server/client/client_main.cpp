#include<iostream>
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

	//ÿ100���볢������
	while (!newclient->Connect("127.0.0.1", 30012))
	{
		delaytime(100);
	}

	std::cout << " connect succeed!" << std::endl;

	MessagePack *recvpack;

	MsgPing msg;
	MsgEnd endmsg;

	int sendnum = 0;
	int64 begin, end;
	begin = get_millisecond();
	//������Ϣ
	newclient->SendMsg(&msg);
	//�����ķ��ͣ�ÿ��sendmsg֮�󣬽��иò����Ż������İ���Ϣ���ͳ�ȥ
	newclient->CheckSend();
	//Ͷ�ݽ�����Ϣ���ò���֮�󣬾Ϳ����յ����Է���˵���Ϣ��
	newclient->CheckRecv();
	sendnum++;

	//��ѭ��
	while (1)
	{
		//���Ի�ȡ����˷�������Ϣ
		recvpack = (MessagePack *)newclient->GetMsg();
		if (recvpack)
		{
			//�յ�����˵���Ϣ��ʱ��
			//������˷���ping��Ϣ
			newclient->SendMsg(&msg);
			newclient->CheckSend();
			sendnum++;
			if (sendnum == 10000)
			{
				//�����ʹ�����1W��ʱ�򣬷���endmsg
				newclient->SendMsg(&endmsg);
				newclient->CheckSend();
				end = get_millisecond();
				std::cout << "end - begin:" << (int)(end - begin) << std::endl;
				break;
			}
		}
		else
		{
			delaytime(0);
		}

		//�����ӶϿ���ʱ������
		if (newclient->IsClose())
			break;

		lxnet::net_run();
	}
	delaytime(1000);

	lxnet::Socketer::Release(newclient);
	lxnet::net_release();
	system("pause");
	return 0;
}