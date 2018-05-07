/*
* 逻辑服务器连接
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ConnectMgr.h"

#define GameConnect CGameConnect::Instance()
class CGameConnect : public CConnectMgr
{
public:
	CGameConnect();
	~CGameConnect();

	static CGameConnect &Instance()
	{
		static CGameConnect m;
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