#include "stdfx.h"
#include "Player.h"
#include "clientmgr.h"
#include "client.h"

extern int64 g_currenttime;
extern bool g_run;

CClientMgr::CClientMgr()
{
	m_ListenPort = 0;
	m_NeedListen = false;
	m_ClientOverTime = 0;
	m_Listen = nullptr;
	m_ClientList.clear();
}

CClientMgr::~CClientMgr()
{
	m_ListenPort = 0;
	m_NeedListen = false;
	m_ClientOverTime = 0;
	m_Listen = nullptr;
	m_ClientList.clear();
}

bool CClientMgr::Init(int port, int clientovertime)
{
	m_ListenPort = port;
	m_ClientOverTime = clientovertime;

	m_Listen = lxnet::Listener::Create();
	if (!m_Listen)
	{
		log_error("create Listener failed!");
		return false;
	}

	m_NeedListen = true;
	return true;
}

void CClientMgr::Release()
{
	for (auto &i : m_ClientList)
	{
		OnClientDisconnect(i);
	}

	lxnet::Listener::Release(m_Listen);
}

void CClientMgr::TestAndListen()
{
	if (!m_NeedListen)
		return ;

	if (!m_Listen->IsClose())
	{
		m_NeedListen = false;
		return;
	}

	if (m_Listen->Listen(m_ListenPort, s_backlog))
	{
		m_NeedListen = false;
		log_error("listen %d succeed!", m_ListenPort);
	}
	else
	{
		log_error("listen %d error!", m_ListenPort);
	}
}

void CClientMgr::AcceptNewClient()
{
	lxnet::Socketer *socket = NULL;

	for (int i = 0; i < s_backlog; ++i)
	{
		if (!m_Listen->CanAccept())
			return;

		if (!(socket = m_Listen->Accept()))
			return;

		//这边可以做一个client pool
		CClient *newclient = new CClient;
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
		m_ClientList.insert(newclient);

		log_error("accept new client, ip:%d", ip);
	}
}

void CClientMgr::ProcessAllClient()
{
	MessagePack *recvpack = NULL;
	std::set<CClient *>::iterator iter, tempiter;
	for (iter = m_ClientList.begin(); iter != m_ClientList.end();)
	{
		tempiter = iter;
		iter ++;

		if ((*tempiter)->bOverTime(g_currenttime,m_ClientOverTime))
		{
			log_error("client is over time! ip: %s", (*tempiter)->GetIP().c_str());
			OnClientDisconnect(*tempiter);
			m_ClientList.erase((*tempiter));
			continue;
		}

		if ((*tempiter)->GetSocket()->IsClose())
		{
			log_error("client is close! ip: %s", (*tempiter)->GetIP().c_str());
			OnClientDisconnect(*tempiter);
			m_ClientList.erase((*tempiter));
			continue;
		}

		(*tempiter)->ProcessMsg();
	}
}

void CClientMgr::SendMsgToAll(Msg *pMsg,CClient *cl)
{
	for (auto &i : m_ClientList)
	{
		if (cl != NULL)
		{
			if(i == cl)
				continue;
		}
		i->SendMsg(pMsg);
	}
}

void CClientMgr::OnClientDisconnect(CClient *cl)
{
	assert(cl != NULL);
	if (cl == NULL)
		return;
	log_error("client disconnect! ip: %s", cl->GetIP().c_str());
	delete cl;
	cl = NULL;
}

void CClientMgr::Run()
{
	TestAndListen();
	AcceptNewClient();
	ProcessAllClient();
}

void CClientMgr::EndRun()
{
	for (auto &i : m_ClientList)
	{
		i->GetSocket()->CheckRecv();
		i->GetSocket()->CheckSend();
	}
}