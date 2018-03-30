#pragma once

#include <string>
#include "lxnet.h"
#include "crosslib.h"
#define MAX_IP_LEN 128
class connector
{
public:
	connector ();
	~connector ();

	void CheckSend ();
	void CheckRecv ();
	bool IsClose ();
	void Close ();
	bool SendMsg (Msg *pMsg, void *adddata = 0, size_t addsize = 0);
	Msg *GetMsg ();
	bool IsAlreadyConnect() { return m_already_connect; }
	bool IsAlreadyRegister() { return m_already_register; }
	void SetAlreadyRegister(bool ready) { m_already_register = ready; }
	void ResetConnect ();
	bool TryConnect(int64 currenttime, const char *ip, int port);
	bool TryConnect(int64 currenttime);
	bool NeedSendPing (int64 currenttime, int64 interval);
	bool IsOverTime (int64 currenttime, int64 overtime);
	void SetRecvPingTime (int64 currenttime);
	void ResetMsgNum ();
	int GetRecvMsgNum () { return m_recvmsgnum; }
	int GetSendMsgNum () { return m_sendmsgnum; }
	void UseBigBuf (bool flag);
	lxnet::Socketer *GetCon () { return m_con; }
	void SetConnectInfo(const char *ip, int port, int id);
	int GetConnectID() { return m_id; }
	const char *GetConnectIP() { return m_ip; }
	void SetConnectName(const char *name) { m_name = name; }
	const char *GetConnectName() { return m_name.c_str(); }
private:
	void CheckAndInit ();
	void Destroy ();
private:
	bool m_isbigbuf;
	bool m_already_connect;
	lxnet::Socketer *m_con;
	int64 m_lastpingtime;
	int64 m_recvpingtime;

	int m_recvmsgnum;
	int m_sendmsgnum;

	char m_ip[MAX_IP_LEN];
	int m_port;
	int m_id;
	bool m_already_register;
	std::string m_name;
};

connector *ConnectorCreate();
void ConnectorRelease(connector *self);
