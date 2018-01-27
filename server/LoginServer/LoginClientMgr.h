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

	void AddClientSecret(int64 clientid, std::string s) { m_CLientSecret[clientid] = s; }
	std::string GetClientSecret(int64 clientid) { auto iter = m_CLientSecret.find(clientid); return  iter == m_CLientSecret.end() ? "" : iter->second; }
private:

	// clientid, 秘钥
	std::unordered_map<int64, std::string> m_CLientSecret;
};