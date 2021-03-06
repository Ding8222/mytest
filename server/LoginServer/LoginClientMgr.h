﻿/*
* Client连接管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ClientMgr.h"

#define LoginClientMgr CLoginClientMgr::Instance()
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

	void Destroy();

	int32 OnNewClient();
	void OnClientDisconnect(CClient *cl);
	void ProcessClientMsg(CClient *cl);
private:

};