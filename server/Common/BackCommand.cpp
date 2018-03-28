#include <limits.h>
#include "BackCommand.h"
#include "tinyxml2.h"
#include "serverlog.h"
#include "msgbase.h"
#include "GlobalDefine.h"

CBackCommand::CBackCommand()
{
	m_LastSend = 0;
	m_ConnectIP.clear();
	m_PingTime = 0;
	m_Name.clear();
	m_DoFunction = NULL;
	m_Listen = NULL;
	m_Sock = NULL;
}

CBackCommand::~CBackCommand()
{
	Destroy();
}

bool CBackCommand::Init(void(*f_dofunc) (lxnet::Socketer *sock), int32 port, int32 pingtime, const char *name)
{
	if (!f_dofunc)
		return false;

	Destroy();
	
	m_DoFunction = f_dofunc;
	m_PingTime = pingtime;
	m_Name = name;

	m_Listen = lxnet::Listener::Create();
	if (!m_Listen)
		return false;

	if (!m_Listen->Listen(port, 1))
	{
		log_error("监听后台端口失败!%d", port);
		return false;
	}
	return true;
}

//在run中执行此函数
void CBackCommand::Run(int64 currenttime)
{
	if (m_Sock && (!m_Sock->IsClose()))
	{
		m_DoFunction(m_Sock);

		if (currenttime > m_LastSend + m_PingTime)
		{
			MessagePack ping;
			ping.PushString("__check_as_ping__");
			m_Sock->SendMsg(&ping);
			m_LastSend = currenttime;
		}

		m_Sock->CheckSend();
		m_Sock->CheckRecv();
		return;
	}

	if (m_Sock)
	{
		BackCommandLog("来自[%s]的连接已断开", m_ConnectIP.c_str());
		m_ConnectIP.clear();

		lxnet::Socketer::Release(m_Sock);
		m_Sock = NULL;
	}
	if (m_Listen->CanAccept())
	{
		m_Sock = m_Listen->Accept();
		if (!m_Sock)
			return;
		m_Sock->SetRecvLimit(enum_sock_buf_data_limit);
		m_Sock->SetSendLimit(enum_sock_buf_data_limit);

		{
			char ip[64];
			m_Sock->GetIP(ip, sizeof(ip) - 1);
			ip[sizeof(ip) - 1] = 0;
			m_ConnectIP = ip;
			BackCommandLog("来自[%s]的新连接", m_ConnectIP.c_str());
		}

		//告知此服的名字
		MessagePack res;
		res.PushString(m_Name.c_str());
		m_Sock->SendMsg(&res);
		m_Sock->CheckSend();
		m_Sock->CheckRecv();
	}
}

void CBackCommand::Destroy()
{
	m_DoFunction = NULL;

	if (m_Listen)
	{
		lxnet::Listener::Release(m_Listen);
		m_Listen = NULL;
	}
	if (m_Sock)
	{
		lxnet::Socketer::Release(m_Sock);
		m_Sock = NULL;
	}
}

