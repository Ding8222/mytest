/*
* Client认证管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <vector>
#include <unordered_map>
#include "msgbase.h"

struct ClientAuthInfo
{
	ClientAuthInfo()
	{
		nLoginSvrID = 0;
		Token.clear();
		Secret.clear();
	}
	
	int32 nLoginSvrID;
	std::string Token;	// 账号
	std::string Secret;	// 秘钥
};

class CClientAuthMgr
{
public:
	CClientAuthMgr();
	~CClientAuthMgr();

	static CClientAuthMgr &Instance()
	{
		static CClientAuthMgr m;
		return m;
	}

	bool Init();
	void Run();
	void Destroy(bool bLoginDisconnect = false);

	// Client请求认证,loginSvr调用
	void QueryAuth(Msg *pMsg, int32 clientid, int32 serverid);
	// 移除认证信息
	void DelClientAuthInfo(int32 clientid);
	ClientAuthInfo *FindClientAuthInfo(int32 clientid);
	int32 GetClientLoginSvr(int32 clientid);
	void SetCenterClientID(const std::string &token,int32 id);
	void ClientOffline(const std::string &token);
private:
	// id,info
	std::vector<ClientAuthInfo *> m_ClientInfoSet;
	// account,clientid
	std::unordered_map<std::string, int32> m_PlayerOnlineMap;
	// account,clientid
	std::unordered_map<std::string, int32> m_PlayerLoginMap;
};