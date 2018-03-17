/*
* 日志服务器连接
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ConnectMgr.h"
#include "msgbase.h"

class CNameCheckConnecter : public CConnectMgr
{
public:
	CNameCheckConnecter();
	~CNameCheckConnecter();

	static CNameCheckConnecter &Instance()
	{
		static CNameCheckConnecter m;
		return m;
	}

	bool Init(const char *ip, int port, int id,
		int serverid, int servertype, int pingtime, int overtime);
	void Destroy();

	void ServerRegisterSucc(int id, const char *ip, int port);
	// 连接断开的处理
	void ConnectDisconnect(connector *);
	// 处理远程服务器发来的消息
	void ProcessMsg(connector *_con);

	bool SendMsgToServer(google::protobuf::Message &pMsg, int maintype, int subtype, int64 nClientID = 0);
	bool SendMsgToServer(Msg &pMsg, int64 nClientID = 0);
private:
	bool isReady;
	int LogServerID;
};
