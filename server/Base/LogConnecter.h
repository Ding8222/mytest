/*
* 日志服务器连接
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ConnectMgr.h"
#include "msgbase.h"

class CLogConnecter : public CConnectMgr
{
public:
	CLogConnecter();
	~CLogConnecter();

	static CLogConnecter &Instance()
	{
		static CLogConnecter m;
		return m;
	}

	bool Init(const char *logserverip, int logserverport, int logserverid, const char *logservername,
		int serverid, int servertype, const char *servername, int pingtime, int overtime);
	void Destroy();

	void ServerRegisterSucc(connector *);
	// 连接断开的处理
	void ConnectDisconnect(connector *);
	// 处理远程服务器发来的消息
	void ProcessMsg(connector *_con);

	// 发送Log
	void SendLog();
private:
	bool isReady;
	int LogServerID;
};
