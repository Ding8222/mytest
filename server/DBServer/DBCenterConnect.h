/*
* 中心服务器连接
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ConnectMgr.h"

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

	// 连接断开的处理
	void ConnectDisconnect(connector *);
	// 处理远程服务器发来的消息
	void ProcessMsg(connector *_con);
	void ProcessServerMsg(connector *_con, Msg *pMsg);
	void ProcessLoginMsg(connector *_con, Msg *pMsg);

private:

};