/*
* Client所在服务器管理
* Copyright (C) ddl
* 2018
*/
#pragma once

#include <list>
struct ClientSvr
{
	ClientSvr()
	{
		nGateID = 0;
		nGameServerID = 0;
	}
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
	void Destroy();

	void AddClientSvr(int32 clientid, int serverid, int servertype);
	void DelClientSvr(int32 clientid);
	ClientSvr *GetClientSvr(int32 id);
	void AddClientLoginSvr(int32 clientid, int serverid);
	void DelClientLoginSvr(int32 clientid);
	int32 GetClientLoginSvr(int32 clientid);
private:

	std::vector<ClientSvr *> m_ClientSvrSet;
	std::vector<int32> m_ClientLoginSvrSet;
};