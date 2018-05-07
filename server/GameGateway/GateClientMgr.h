/*
* Client连接管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ClientMgr.h"

#define GateClientMgr CGateClientMgr::Instance()
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

	int32 OnNewClient();
	void OnClientDisconnect(CClient *cl);
	void ReleaseAllClient();
	void ProcessClientMsg(CClient *cl);
	void ProcessClientAuth(CClient *cl, Msg *pMsg);
	void SetClientAlreadyLogin(int32 clientid, int32 gameserverid);
private:

};