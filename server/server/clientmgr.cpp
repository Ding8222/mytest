#include "stdfx.h"

extern int64 g_currenttime;
extern bool g_run;

clientmgr::clientmgr()
{
	m_listen_port = 0;
	m_needlisten = false;
	m_clientovertime = 0;
	m_listen = NULL;
	m_client_list.clear();
}

clientmgr::~clientmgr()
{
	m_listen_port = 0;
	m_needlisten = false;
	m_clientovertime = 0;
	m_listen = NULL;
	m_client_list.clear();
}

bool clientmgr::init(int port, int clientovertime)
{
	m_listen_port = port;
	m_clientovertime = clientovertime;

	m_listen = lxnet::Listener::Create();
	if (!m_listen)
	{
		std::cout << "create Listener failed!" << std::endl;
		return false;
	}

	m_needlisten = true;
	return true;
}

void clientmgr::release()
{
	for (auto &i : m_client_list)
	{
		OnClientDisconnect(i);
	}

	lxnet::Listener::Release(m_listen);
}

void clientmgr::testandlisten()
{
	if (!m_needlisten)
		return ;

	if (!m_listen->IsClose())
	{
		m_needlisten = false;
		return;
	}

	if (m_listen->Listen(m_listen_port, s_backlog))
	{
		m_needlisten = false;
		std::cout << "listen "<< m_listen_port <<" succeed!" << std::endl;
	}
	else
	{
		std::cout << "listen " << m_listen_port << " error!" << std::endl;
	}
}

void clientmgr::acceptnewclient()
{
	lxnet::Socketer *socket = NULL;

	for (int i = 0; i < s_backlog; ++i)
	{
		if (!m_listen->CanAccept())
			return;

		if (!(socket = m_listen->Accept()))
			return;

		//这边可以做一个client pool
		client *newclient = new client;
		assert(newclient != NULL);
		if (!newclient)
		{
			return;
		}

 		//启用压缩
 		socket->UseCompress();
 		//启用解密
		socket->UseDecrypt();
 		//启用加密
 		socket->UseEncrypt();
// 		//设置接手数据字节的临界值
// 		socket->SetRecvLimit(16 * 1024);
		//设置发送数据字节的临界值
		socket->SetSendLimit(-1);
		char ip[128];
		socket->GetIP(ip, sizeof(ip) - 1);

		newclient->SetSocket(socket);
		newclient->SetConnectTime(g_currenttime);
		newclient->SetPingTime(g_currenttime);
		newclient->SetIP(ip);
		m_client_list.insert(newclient);

		std::cout << "accept new client, ip:" << ip << std::endl;
	}
}

void clientmgr::processallclient()
{
	MessagePack *recvpack = NULL;
	std::set<client *>::iterator iter, tempiter;
	for (iter = m_client_list.begin(); iter != m_client_list.end();)
	{
		tempiter = iter;
		iter ++;

		if ((*tempiter)->bOverTime(g_currenttime,m_clientovertime))
		{
			std::cout << "client is over time! ip:" << (*tempiter)->GetIP() << std::endl;
			OnClientDisconnect(*tempiter);
			m_client_list.erase((*tempiter));
			continue;
		}

		if ((*tempiter)->GetSocket()->IsClose())
		{
			std::cout << "client is close! ip:" << (*tempiter)->GetIP() << std::endl;
			OnClientDisconnect(*tempiter);
			m_client_list.erase((*tempiter));
			continue;
		}

		processclientmsg(*tempiter);
	}
}

void clientmgr::processclientmsg(client *cl)
{
	assert(cl != NULL);
	if (cl == NULL)
		return;

	Msg *pMsg = NULL;
	while (1)
	{
		pMsg = cl->GetMsg();
		if (!pMsg)
			break;
		switch (pMsg->GetType())
		{
		case MSG_PING:
		{
			//收到ping消息的时候，也发送ping消息给前端，然后设置ping消息的接受时间
			MsgPing* msg = (MsgPing*)pMsg;
			if (msg)
			{
				msg->m_servertime = g_currenttime;
				cl->SendMsg(pMsg);
				cl->SetPingTime(g_currenttime);
			}
			break;
		}
		case MSG_END:
		{
			std::cout << cl->GetIP() << ":get end msg" << std::endl;
			break;
		}
		case MSG_CHAT:
		{
			//把收到的chat消息发给所有在线的client
			MessagePack * msg = (MessagePack *)pMsg;
			char stBuff[521] = { 0 };
			msg->Begin();
			msg->GetString(stBuff, 512);
			std::cout << cl->GetIP() << ":" << stBuff << std::endl;
			SendMsgToAll(msg);
			break;
		}
		case MSG_LOAD:
		{
			//加载
			MessagePack * msg = (MessagePack *)pMsg;
			msg->Begin();
			char sName[48];
			msg->GetString(sName,48);
			playerobj * player = new playerobj;
			cl->SetPlayer(player);
			if (!player->load(sName, scenemgr::Instance().getscene(1), cl))
			{
				std::cout << "Player " << sName << " Load Faild" << std::endl;
			}
			break;
		}
		case MSG_MOVE:
		{
			//移动
			MessagePack * msg = (MessagePack *)pMsg;
			msg->Begin();
			float _Pos[3] = {0};
			_Pos[0] = msg->GetFloat();
			_Pos[1] = msg->GetFloat();
			_Pos[2] = msg->GetFloat();
			playerobj * player = cl->GetPlayer();
			player->moveto(_Pos[0], _Pos[1], _Pos[2]);
			break;
		}
		default:
		{
			std::cout << "msg type error!" << std::endl;
		}
		}
	}
}

void clientmgr::SendMsgToAll(Msg *pMsg,client *cl)
{
	for (auto &i : m_client_list)
	{
		if (cl != NULL)
		{
			if(i == cl)
				continue;
		}
		i->SendMsg(pMsg);
	}
}

void clientmgr::OnClientDisconnect(client *cl)
{
	assert(cl != NULL);
	if (cl == NULL)
		return;
	std::cout << "client disconnect! ip:" << cl->GetIP() << std::endl;
	cl->~client();
	delete cl;
	cl = NULL;
}

void clientmgr::run()
{
	testandlisten();
	acceptnewclient();

	processallclient();
}

void clientmgr::endrun()
{
	for (auto &i : m_client_list)
	{
		i->GetSocket()->CheckRecv();
		i->GetSocket()->CheckSend();
	}
}