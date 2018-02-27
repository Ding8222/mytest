#include "Connector.h"
#include "serverlog.h"
#include "msgbase.h"
#include "GlobalDefine.h"
#include "google/protobuf/message.h"
#include "Robot.h"

#include "MainType.h"
#include "ServerType.h"
#include "ServerMsg.pb.h"

extern int64 g_currenttime;

CRobot::CRobot()
{
	sSecret.clear();
	sAccount.clear();
	m_isAuth = false;
	m_isHandShake = false;
	m_TempID = 0;
}

CRobot::~CRobot()
{
	Destroy();
}

void CRobot::SendMsg(google::protobuf::Message &pMsg, int maintype, int subtype, void *adddata, size_t addsize)
{
	MessagePack pk;
	pk.Pack(&pMsg, maintype, subtype);
	connector::SendMsg(&pk, adddata, addsize);
}

void CRobot::OnConnectDisconnect()
{
	if (IsAlreadyRegister())
	{

	}

	ResetConnect();
	SetAlreadyRegister(false);
}

void CRobot::ChangeConnect(const char *ip, int port, int id, bool bauth)
{
	m_isAuth = bauth;
	SetConnectInfo(ip,port,id);
	OnConnectDisconnect();
}

void CRobot::Destroy()
{

}

void CRobot::ProcessRegister(connector *con)
{
	MessagePack *pMsg = (MessagePack *)con->GetMsg();

	//未注册，则一帧只处理一个消息。
	if (pMsg)
	{
		switch (pMsg->GetMainType())
		{
		case SERVER_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case SVR_SUB_PING:
			{
				con->SetRecvPingTime(g_currenttime);
				break;
			}
			case SVR_SUB_SERVER_REGISTER_RET:
			{
				svrData::ServerRegisterRet msg;
				_CHECK_PARSE_(pMsg, msg);

				switch (msg.nretcode())
				{
				case svrData::ServerRegisterRet::EC_SUCC:
				{
					// 认证成功
					con->SetAlreadyRegister(true);
					RunStateLog("注册到远程服务器成功！");
					break;
				}
				case svrData::ServerRegisterRet::EC_SERVER_ID_EXIST:
				{
					// 已存在相同ServerID被注册
					RunStateError("注册到远程服务器失败！已存在相同ServerID被注册，远程服务器ID：[%d] IP:[%s]", GetConnectID(), GetConnectIP());
					exit(-1);
					break;
				}
				case svrData::ServerRegisterRet::EC_TO_CONNECT_ID_NOT_EQUAL:
				{
					// 请求注册的ServerID和远程ServerID不同
					RunStateError("注册到远程服务器失败！请求注册的ServerID和远程ServerID不同，远程服务器ID：[%d] IP:[%s]", GetConnectID(), GetConnectIP());
					exit(-1);
					break;
				}
				}
				break;
			}
			default:
				break;
			}
			break;
		}
		}
	}
}

void CRobot::ProcessMsg(connector *_con)
{
	Msg *pMsg = NULL;
	for (;;)
	{
		pMsg = _con->GetMsg();
		if (!pMsg)
			break;
		switch (pMsg->GetMainType())
		{
		case SERVER_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case SVR_SUB_PING:
			{
				_con->SetRecvPingTime(g_currenttime);
				break;
			}
			default:
			{
			}
			}
			break;
		}
		default:
		{
		}
		}
	}
}

// #include<iostream>
// #include<thread>//c++ 11多线程
// #include<string>
// #include "lxnet.h"
// #include "crosslib.h"
// #include "msgdef.h"
// 
// #ifdef _WIN32
// #include <windows.h>
// #include "MiniDump.h"
// 
// #define delaytime(v)	Sleep(v)
// #else
// #include <unistd.h>
// 
// #define delaytime(v)	usleep(v * 1000)
// #define system(a)
// #endif
// 
// static bool g_run;
// 
// //每5秒给server发送一个ping
// void PingMsg(lxnet::Socketer *clientsocket)
// {
// 	MsgPing msg;
// 	while (g_run)
// 	{
// 		if (!clientsocket || clientsocket->IsClose())
// 		{
// 			g_run = false;
// 			break;
// 		}
// 		clientsocket->SendMsg(&msg);
// 		delaytime(5000);
// 	}
// 	std::cout << "Ping task end!" << std::endl;
// }
// 
// //用于用户输入
// void InputMsg(lxnet::Socketer *clientsocket)
// {
// 	MessagePack msg;
// 	std::string str;
// 	while (g_run)
// 	{
// 		if (!clientsocket || clientsocket->IsClose())
// 		{
// 			g_run = false;
// 			break;
// 		}
// 
// 		std::cout << "请输入命令：" << std::endl;
// 		std::cin >> str;
// 
// 		if (str == "quit")
// 		{
// 			g_run = false;
// 			break;
// 		}
// 
// 		msg.Reset();
// 		msg.SetType(MSG_CHAT);
// 		msg.PushString(str.c_str());
// 
// 		if (str == "load")
// 		{
// 			std::cout << "请输入角色名：" << std::endl;
// 			std::cin >> str;
// 			msg.Reset();
// 			msg.SetType(MSG_LOAD);
// 			msg.PushString(str.c_str());
// 		}
// 		else if (str == "move")
// 		{
// 			msg.Reset();
// 			msg.SetType(MSG_MOVE);
// 			std::cout << "请输入x坐标：" << std::endl;
// 			std::cin >> str;
// 			msg.PushFloat(atof(str.c_str()));
// 			std::cout << "请输入y坐标：" << std::endl;
// 			std::cin >> str;
// 			msg.PushFloat(atof(str.c_str()));
// 			std::cout << "请输入z坐标：" << std::endl;
// 			std::cin >> str;
// 			msg.PushFloat(atof(str.c_str()));
// 		}
// 
// 		clientsocket->SendMsg(&msg);
// 
// 		delaytime(10);
// 	}
// 	std::cout << "Input task end!" << std::endl;
// }
// 
// void PrintMsg(lxnet::Socketer *clientsocket)
// {
// 	MessagePack *recvpack = NULL;
// 	while (g_run)
// 	{
// 		if (!clientsocket || clientsocket->IsClose())
// 		{
// 			g_run = false;
// 			break;
// 		}
// 
// 		recvpack = (MessagePack *)clientsocket->GetMsg();
// 		if (recvpack)
// 		{
// 			switch (recvpack->GetType())
// 			{
// 			case MSG_PING:
// 			{
// 				//recvpack->Begin();
// 				//MsgPing *msg = (MsgPing *)recvpack;
// 				//std::cout << "Server Time :" << msg->m_servertime << std::endl;
// 				break;
// 			}
// 			case MSG_CHAT:
// 			{
// 				char stBuff[521] = { 0 };
// 				recvpack->Begin();
// 				recvpack->GetString(stBuff, 512);
// 				std::cout << "Get Msg From Server: " << stBuff << std::endl;
// 				break;
// 			}
// 			case MSG_ENTER:
// 			{
// 				char stBuff[48] = { 0 };
// 				recvpack->Begin();
// 				recvpack->GetString(stBuff, 512);
// 				std::cout << "Player:" << stBuff << " Enter View" << std::endl;
// 				break;
// 			}
// 			default:
// 			{
// 
// 			}
// 			}
// 		}
// 		else
// 			delaytime(10);
// 	}
// 	std::cout << "Print task end!" << std::endl;
// }
// 
// int main(void)
// {
// #ifdef _WIN32
// 	if (!CMiniDump::Begin())
// 	{
// 		std::cout << "初始化MiniDump失败!" << std::endl;
// 		system("pause");
// 		return 0;
// 	}
// #endif
// 
// 	//初始化网络库
// 	if (!lxnet::net_init(512, 1, 1024 * 32, 100, 1, 4, 1))
// 	{
// 		std::cout << "init network error!" << std::endl;
// 		system("pause");
// 		return 0;
// 	}
// 
// 	//创建socket
// 	lxnet::Socketer *newclient = lxnet::Socketer::Create();
// 
// 	if (!newclient)
// 	{
// 		std::cout << "create client socketer error!" << std::endl;
// 		system("pause");
// 		return 0;
// 	}
// 
// 	//每100毫秒尝试连接
// 	while (!newclient->Connect("127.0.0.1", 30012))
// 	{
// 		delaytime(100);
// 	}
// 
// 	std::cout << "connect succeed!" << std::endl;
// 
// 	//启用解压缩
// 	newclient->UseUncompress();
// 	//启用加密
// 	newclient->UseEncrypt();
// 	//启用解密
// 	newclient->UseDecrypt();
// 	//尝试投递接取消息
// 	newclient->CheckRecv();
// 
// 	g_run = true;
// 
// 	//创建三个线程
// 	//心跳
// 	std::thread PingTask(PingMsg, newclient);
// 	//输入
// 	std::thread InputTask(InputMsg, newclient);
// 	//将线程分离
// 	InputTask.detach();
// 	//接受输出
// 	std::thread PrintTask(PrintMsg, newclient);
// 
// 	//死循环
// 	while (g_run)
// 	{
// 		lxnet::net_run();
// 		if (!newclient || newclient->IsClose())
// 			break;
// 
// 		delaytime(100);
// 
// 		newclient->CheckSend();
// 	}
// 
// 	//等待线程结束--阻塞
// 	PingTask.join();
// 	PrintTask.join();
// 
// 	delaytime(1000);
// 
// 	lxnet::Socketer::Release(newclient);
// 	lxnet::net_release();
// 	system("pause");
// 
// #ifdef _WIN32
// 	CMiniDump::End();
// #endif
// 	return 0;
// }