#include "connector.h"
#include "objectpool.h"
#include "log.h"

connector::connector ()
{
	memset(m_ip, 0, MAX_IP_LEN);
	m_port = 0;
	m_id = 0;
	m_isready = false;

	m_isbigbuf = true;
	m_already_connect = false;
	m_con = NULL;
	m_lastpingtime = 0;
	m_recvpingtime = 0;

	m_recvmsgnum = 0;
	m_sendmsgnum = 0;
}

connector::~connector ()
{
	Destroy();
}

void connector::CheckSend ()
{
	CheckAndInit();
	m_con->CheckSend();
}

void connector::CheckRecv ()
{
	CheckAndInit();
	m_con->CheckRecv();
}

bool connector::IsClose ()
{
	CheckAndInit();
	return m_con->IsClose();
}

void connector::Close ()
{
	m_con->Close();
}

bool connector::SendMsg (Msg *pMsg, void *adddata, size_t addsize)
{
	if (!pMsg)
		return false;
	++m_sendmsgnum;
	CheckAndInit();
	return m_con->SendMsg(pMsg, adddata, addsize);
}

Msg *connector::GetMsg ()
{
	CheckAndInit();
	Msg *pMsg = m_con->GetMsg();
	if (!pMsg)
		return NULL;
	++m_recvmsgnum;
	return pMsg;
}

void connector::ResetConnect ()
{
	if (m_con)
	{
		lxnet::Socketer::Release(m_con);
		m_con = NULL;
	}
	m_con = lxnet::Socketer::Create(m_isbigbuf);
	m_already_connect = false;
	m_isready = false;
}

bool connector::TryConnect (int64 currenttime, const char *ip, int port)
{
	CheckAndInit();
	if (m_con->Connect(ip, port))
	{
		m_already_connect = true;
		SetRecvPingTime(currenttime);
		return true;
	}

	return false;
}

bool connector::TryConnect(int64 currenttime)
{
	CheckAndInit();
	if (m_con->Connect(m_ip, m_port))
	{
		m_already_connect = true;
		SetRecvPingTime(currenttime);
		return true;
	}

	return false;
}

bool connector::NeedSendPing (int64 currenttime, int64 interval)
{
	if ((currenttime - m_lastpingtime) >= interval)
	{
		m_lastpingtime = currenttime;
		return true;
	}

	return false;
}

bool connector::IsOverTime (int64 currenttime, int64 overtime)
{
	return ((currenttime - m_recvpingtime) >= overtime);
}

void connector::SetRecvPingTime (int64 currenttime)
{
	m_recvpingtime = currenttime;
}

void connector::ResetMsgNum ()
{
	m_recvmsgnum = 0;
	m_sendmsgnum = 0;
}

void connector::UseBigBuf (bool flag)
{
	m_isbigbuf = flag;
}

void connector::CheckAndInit ()
{
	if (!m_con)
		ResetConnect();
}

void connector::Destroy ()
{
	if (m_con)
	{
		lxnet::Socketer::Release(m_con);
		m_con = NULL;
	}
	m_already_connect = false;
}

void connector::SetConnectInfo(const char *ip, int port, int id)
{
	strncpy(m_ip, ip, MAX_IP_LEN - 1);
	m_ip[MAX_IP_LEN - 1] = 0;
	m_port = port;
	m_id = id;
}

static objectpool<connector> &ConnectorPool()
{
	static objectpool<connector> m(128, "connector pools");
	return m;
}

connector *ConnectorCreate()
{
	connector *self = ConnectorPool().GetObject();
	if (!self)
	{
		log_error("创建 connector 失败!");
		return NULL;
	}
	new(self) connector();
	return self;
}

void ConnectorRelease(connector *self)
{
	if (!self)
		return;
	self->~connector();
	ConnectorPool().FreeObject(self);
}