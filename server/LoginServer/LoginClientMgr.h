/*
* Client连接管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ClientMgr.h"

class CLoginClientMgr : public CClientMgr
{
public:
	CLoginClientMgr();
	~CLoginClientMgr();

	static CLoginClientMgr &Instance()
	{
		static CLoginClientMgr m;
		return m;
	}

	int64 OnNewClient();
	void OnClientDisconnect(CClient *cl);
	void ProcessClientMsg(CClient *cl);

private:

};