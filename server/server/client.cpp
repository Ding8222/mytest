#include "stdfx.h"

client::client()
{
	m_connecttime = 0;
	m_pingtime = 0;
	m_socket = NULL;
	memset(m_ip, 0, sizeof(m_ip));
}

client::~client()
{
	m_connecttime = 0;
	m_pingtime = 0;
	if (m_socket)
	{
		lxnet::Socketer::Release(m_socket);
		m_socket = NULL;
	}
	memset(m_ip, 0, sizeof(m_ip));
}

bool client::bOverTime(int64 currenttime, int clientovertime)
{
	if (clientovertime > 0)
	{
		if (currenttime >= m_pingtime + clientovertime)
		{
			return true;
		}
	}

	return false;
}

void client::SetSocket(lxnet::Socketer *socket)
{
	m_socket = socket;
}

void client::SendMsg(Msg *pMsg)
{
	assert(pMsg != NULL);
	if (!pMsg)
		return;
	assert(m_socket != NULL);
	if (!m_socket)
		return ;
	m_socket->SendMsg(pMsg);
}

Msg *client::GetMsg()
{
	assert(m_socket != NULL);
	if (!m_socket)
		return NULL;
	Msg *msg = m_socket->GetMsg();
	if (!msg)
		return NULL;

	return msg;
}