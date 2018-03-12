/*
* Client认证管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <vector>
#include "msgbase.h"

struct idmgr;
struct ClientAuthInfo
{
	ClientAuthInfo()
	{
		nClientID = 0;
		nLoginSvrID = 0;
		Token.clear();
		Secret.clear();
	}
	
	int32 nClientID;
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
	void AddClientAuthInfo(Msg *pMsg, int32 clientid, int32 serverid);
	// 移除认证信息
	void DelClientAuthInfo(int32 clientid);
	ClientAuthInfo *FindClientAuthInfo(int32 clientid);
	int32 GetClientLoginSvr(int32 clientid);
private:
	// id,info
	std::vector<ClientAuthInfo *> m_ClientInfoSet;
	idmgr *m_IDPool;

};