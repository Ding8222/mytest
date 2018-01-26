/*
* 逻辑服务器连接
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ServerConnect.h"

class CGameConnect : public CServerConnect
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

	// 连接断开的处理
	void ConnectDisconnect();
	// 处理远程服务器发来的消息
	void ProcessMsg(connector *_con);
private:

};