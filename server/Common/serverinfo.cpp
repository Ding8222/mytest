#include <assert.h>
#include <string.h>
#include "serverinfo.h"
#include "log.h"
#include "objectpool.h"

serverinfo::serverinfo ()
{
	m_already_register = false;
	m_serverid = 0;
	m_servertype = 0;
	m_removetime = 0;
	m_connecttime = 0;
	m_pingtime = 0;

	m_recvmsg_num = 0;
	m_sendmsg_num = 0;

	memset(m_ip, 0, sizeof(m_ip));
	m_con = NULL;
	m_servername.clear();
}

serverinfo::~serverinfo ()
{
	if (m_con)
	{
		lxnet::Socketer::Release(m_con);
		m_con = NULL;
	}
	m_servername.clear();
}

void serverinfo::SetCon (lxnet::Socketer *sock)
{
	m_con = sock;
}

void serverinfo::SendMsg (Msg *pMsg, void *adddata, size_t addsize)
{
	assert(pMsg != NULL);
	m_con->SendMsg(pMsg, adddata, addsize);
	++m_sendmsg_num;
}

Msg *serverinfo::GetMsg ()
{
	Msg *pMsg = NULL;
	pMsg = m_con->GetMsg();
	if (pMsg)
		++m_recvmsg_num;
	return pMsg;
}

void serverinfo::SetServerID (int32 serverid)
{
	m_serverid = serverid;
}

void serverinfo::SetServerType (int32 servertype)
{
	m_servertype = servertype;
}

void serverinfo::SetConnectTime (int64 currenttime)
{
	m_connecttime = currenttime + enum_server_not_register_id_overtime;
}

bool serverinfo::IsEnoughXMNotReg (int64 currenttime)
{
	if (m_already_register)
		return false;
	return (currenttime >= m_connecttime);
}

bool serverinfo::IsOverTime (int64 currenttime, int32 overtime)
{
	return (currenttime - m_pingtime >= overtime);
}

void serverinfo::SetRemove (int64 currenttime)
{
	m_removetime = currenttime + enum_server_remove_delay_time;
}

bool serverinfo::CanRemove (int64 currenttime)
{
	if (!IsNeedRemove())
		return false;
	return (currenttime >= m_removetime);
}

void serverinfo::ResetMsgNum ()
{
	m_recvmsg_num = 0;
	m_sendmsg_num = 0;
}

void serverinfo::SetIP (const char *ip)
{
	if (!ip)
		return;
	strncpy(m_ip, ip, sizeof(m_ip)- 1);
	m_ip[sizeof(m_ip) - 1] = '\0';
}

static objectpool<serverinfo> &ServerInfoPool ()
{
	static objectpool<serverinfo> m(128, "serverinfo pools");
	return m;
}

serverinfo *serverinfo_create ()
{
	serverinfo *self = ServerInfoPool().GetObject();
	if (!self)
	{
		log_error("创建 serverinfo 失败!");
		return NULL;
	}
	new(self) serverinfo();
	return self;
}

void serverinfo_release (serverinfo *self)
{
	if (!self)
		return;
	self->~serverinfo();
	ServerInfoPool().FreeObject(self);
}

