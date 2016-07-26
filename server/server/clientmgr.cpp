#include"stdfx.h"
#include"msgbase.h"

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

		//��߿�����һ��client pool
		client *newclient = new client;
		assert(newclient != NULL);
		if (!newclient)
		{
			return;
		}

 		//����ѹ��
 		socket->UseCompress();
 		//���ý���
		socket->UseDecrypt();
 		//���ü���
 		socket->UseEncrypt();
// 		//���ý��������ֽڵ��ٽ�ֵ
// 		socket->SetRecvLimit(16 * 1024);
		//���÷��������ֽڵ��ٽ�ֵ
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
			//�յ�ping��Ϣ��ʱ��Ҳ����ping��Ϣ��ǰ�ˣ�Ȼ������ping��Ϣ�Ľ���ʱ��
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
			//���յ���chat��Ϣ�����������ߵ�client
			MessagePack * msg = (MessagePack *)pMsg;
			char stBuff[521] = { 0 };
			msg->Begin();
			msg->GetString(stBuff, 512);
			std::cout << cl->GetIP() << ":" << stBuff << std::endl;
			SendMsgToAll(msg);
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