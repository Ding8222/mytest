/*
* 中心服务器连接
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ConnectMgr.h"
#include "sqlinterface.h"

class CDBCenterConnect : public CConnectMgr
{
public:
	CDBCenterConnect();
	~CDBCenterConnect();

	static CDBCenterConnect &Instance()
	{
		static CDBCenterConnect m;
		return m;
	}

	bool Init();

	void ServerRegisterSucc(int id, const char *ip, int port);
	// 连接断开的处理
	void ConnectDisconnect(connector *);
	// 处理远程服务器发来的消息
	void ProcessMsg(connector *_con);
	void ProcessServerMsg(connector *_con, Msg *pMsg, msgtail *tl);
private:

};

extern DataBase::CConnection g_dbhand;
