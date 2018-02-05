/*
* Client所在服务器管理
* Copyright (C) ddl
* 2018
*/
#pragma once

struct ClientSvr
{
	ClientSvr()
	{
		nClientID = 0;
		nLoginServerID = 0;
		nGateID = 0;
		nGameServerID = 0;
	}
	int64 nClientID;
	int nLoginServerID;
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

	void AddClientSvr(int64 clientid, int serverid, int servertype);
	void DelClientSvr(int64 id);
	ClientSvr *GetClientSvr(int64 id);
private:

	std::unordered_map<int64, ClientSvr> m_ClientSvr;
};