#pragma once

#include "lxnet.h"
#include "crosslib.h"
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
	bool IsAlreadyConnect () { return m_already_connect; }
	void ResetConnect ();
	bool TryConnect (int64 currenttime, const char *ip, int port);
	bool NeedSendPing (int64 currenttime, int64 interval);
	bool IsOverTime (int64 currenttime, int64 overtime);
	void SetRecvPingTime (int64 currenttime);
	void ResetMsgNum ();
	int GetRecvMsgNum () { return m_recvmsgnum; }
	int GetSendMsgNum () { return m_sendmsgnum; }
	void UseBigBuf (bool flag);
	lxnet::Socketer *GetCon () { return m_con; }
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
};
