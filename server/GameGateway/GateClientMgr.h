/*
* Client连接管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ClientMgr.h"

class CGateClientMgr : public CClientMgr
{
public:
	CGateClientMgr();
	~CGateClientMgr();

	static CGateClientMgr &Instance()
	{
		static CGateClientMgr m;
		return m;
	}

	void Destroy();

	int64 OnNewClient();
	void OnClientDisconnect(CClient *cl);
	void ProcessClientMsg(CClient *cl);
	void ProcessClientAuth(CClient *cl, Msg *pMsg);
	void SetClientAlreadyLogin(int64 clientid, bool bLogin);
private:

};