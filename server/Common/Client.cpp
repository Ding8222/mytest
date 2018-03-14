#include <assert.h>
#include <string>
#include "Client.h"

int g_SendToClientNum = 0;
int g_RecvFromClientNum = 0;

CClient::CClient()
{
	m_AlreadyLogin = false;
	m_AlreadyAuth = false;
	m_ConnectTime = 0;

	m_InlistState = enum_inlist_state_unknow;
	m_ClientID = 0;
	m_PingTime = 0;
	m_RemoveTime = 0;
	m_con = NULL;

	m_PlayerOnlyID = 0;
	m_GameServerID = 0;
}

CClient::~CClient()
{
	if (m_con)
	{
		lxnet::Socketer::Release(m_con);
		m_con = NULL;
	}
}

void CClient::SetClientID(int id)
{
	m_ClientID = id;
}

void CClient::SetCon(lxnet::Socketer *con)
{
	m_con = con;
}

void CClient::SendMsg(Msg *pMsg)
{
	assert(pMsg != NULL);
	if (!pMsg)
		return;
	++g_SendToClientNum;
	assert(m_con != NULL);
	m_con->SendMsg(pMsg);
}

Msg *CClient::GetMsg()
{
	Msg *pMsg = m_con->GetMsg();
	if (!pMsg)
		return NULL;

	++g_RecvFromClientNum;
	return pMsg;
}

void CClient::SetRemove(int64 currenttime)
{
	m_RemoveTime = currenttime + enum_remove_delay_time;
}

bool CClient::CanRemove(int64 currenttime)
{
	if (!IsNeedRemove())
		return false;
	return (currenttime >= m_RemoveTime);
}

bool CClient::IsOverTime(int64 currenttime, int clientovertime)
{
	if (currenttime - m_PingTime >= clientovertime)
		return true;
	else
		return false;
}

void CClient::SetConnectTime(int64 currenttime)
{
	m_ConnectTime = currenttime + enum_not_authsucceed_max_time;
}

bool CClient::IsEnoughXMNotAlreadyAuth(int64 currenttime)
{
	if (m_AlreadyAuth)
		return false;
	return currenttime >= m_ConnectTime;
}
