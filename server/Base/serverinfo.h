#pragma once
#include <string>
#include "lxnet.h"
#include "crosslib.h"

struct Msg;
class serverinfo
{
public:
	enum
	{
		enum_server_remove_delay_time = 5000,			//处于延时关闭列时，延时*ms再移除
		enum_server_not_register_id_overtime = 5000,	//连接上来后，若*ms，还未注册类型以及id，则关掉它
	};
	serverinfo ();
	~serverinfo ();
	void SetCon (lxnet::Socketer *sock);
	lxnet::Socketer *GetCon () { return m_con; }
	void SendMsg (Msg *pMsg, void *adddata = NULL, size_t addsize = 0);
	Msg *GetMsg();
	void SetServerID (int32 serverid);
	int32 GetServerID () { return m_serverid; }
	void SetServerType (int32 servertype);
	int32 GetServerType () { return m_servertype; }
	void SetConnectTime (int64 currenttime);
	bool IsEnoughXMNotReg (int64 currenttime);
	void SetNotRegister () { m_already_register = false; }
	void SetAlreadyRegister () { m_already_register = true; }
	bool IsAlreadyRegister () { return m_already_register; }
	void SetPingTime (int64 currenttime) { m_pingtime = currenttime; }
	bool IsOverTime (int64 currenttime, int32 overtime);
	bool IsNeedRemove () { return (m_removetime != 0); }
	void SetRemove (int64 currenttime);
	bool CanRemove (int64 currenttime);
	void ResetMsgNum ();
	int32 GetRecvMsgNum () { return m_recvmsg_num; }
	int32 GetSendMsgNum () { return m_sendmsg_num; }
	const char *GetIP () { return m_ip; }
	void SetIP (const char *ip);
	void SetServerName(const char *name) { m_servername = name; }
	const char *GetServerName() { return m_servername.c_str(); }
private:
	bool m_already_register;
	int32 m_serverid;
	int32 m_servertype;
	int64 m_removetime;
	int64 m_connecttime;
	int64 m_pingtime;
	
	int32 m_recvmsg_num;
	int32 m_sendmsg_num;

	char m_ip[64];
	std::string m_servername;
	lxnet::Socketer *m_con;
};

serverinfo *serverinfo_create ();
void serverinfo_release (serverinfo *self);
