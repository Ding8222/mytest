/*
* Client所在服务器管理
* Copyright (C) ddl
* 2018
*/
#pragma once

#include <list>
struct idmgr;
struct ClientSvr
{
	ClientSvr()
	{
		nClientID = 0;
		nGateID = 0;
		nGameServerID = 0;
	}

	int nClientID;
	int nGateID;
	int nGameServerID;
};

class CClientSvrMgr
{
public:
	CClientSvrMgr();
	~CClientSvrMgr();


	static CClientSvrMgr &Instance()
	{
		static CClientSvrMgr m;
		return m;
	}
	bool Init();
	void Run();
	void Destroy();

	int32 AddClientSvr(int32 clientid, int32 serverid, int32 gateid);
	void UpdateClientGameSvr(int32 clientid, int32 serverid);
	void DelClientSvr(int32 clientid);
	ClientSvr *GetClientSvr(int32 id);
private:

	std::vector<ClientSvr *> m_ClientSvrSet;
	idmgr *m_IDPool;
};