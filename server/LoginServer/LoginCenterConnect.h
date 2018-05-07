/*
* 中心服务器连接
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ConnectMgr.h"

#define LoginCenterConnect CLoginCenterConnect::Instance()
class CLoginCenterConnect : public CConnectMgr
{
public:
	CLoginCenterConnect();
	~CLoginCenterConnect();

	static CLoginCenterConnect &Instance()
	{
		static CLoginCenterConnect m;
		return m;
	}

	bool Init();
	void Destroy();

	// 连接断开的处理
	void ConnectDisconnect(connector *);
	// 处理远程服务器发来的消息
	void ProcessMsg(connector *_con);
private:

};