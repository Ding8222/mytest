/*
* 中心服务器连接
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ConnectMgr.h"

class CGateCenterConnect : public CConnectMgr
{
public:
	CGateCenterConnect();
	~CGateCenterConnect();

	static CGateCenterConnect &Instance()
	{
		static CGateCenterConnect m;
		return m;
	}

	bool Init();
	void Destroy();

	void ServerRegisterSucc(connector *);
	// 连接断开的处理
	void ConnectDisconnect(connector *);
	// 处理远程服务器发来的消息
	void ProcessMsg(connector *_con);

private:

};