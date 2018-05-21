/*
* 中心服务器连接
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ConnectMgr.h"
#include "sqlinterface.h"
#include "msgbase.h"

class datahand;
#define DBCenterConnect CDBCenterConnect::Instance()
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
	void Run();
	void Destroy();
	void GetDataHandInfo(char *buf, size_t buflen);

	// 连接断开的处理
	void ConnectDisconnect(connector *);
	// 处理远程服务器发来的消息
	void ProcessMsg(connector *_con);
public:
	void AddNewTask(Msg *pMsg, int serverid, int tasktype, bool sendtoall = false);
private:

	datahand * m_Hand;
};

