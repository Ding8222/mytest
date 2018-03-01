/*
* Client认证管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <vector>
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
	void Destroy();

	// Client请求认证,loginSvr调用
	void AddClientAuthInfo(Msg *pMsg, int32 clientid, int32 serverid);
	// 移除认证信息
	void DelClientAuthInfo(int32 clientid);
	ClientAuthInfo *FindClientAuthInfo(int32 clientid);
	int32 GetClientLoginSvr(int32 clientid);
	// 发送认证信息到逻辑服
	void SendAuthInfoToLogic(Msg *pMsg, int32 clientid);
	// 发送加载玩家数据到逻辑服
	void SendLoadPlayerDataToLogic(Msg *pMsg, int32 clientid);
private:
	// id,info
	std::vector<ClientAuthInfo *> m_ClientInfoSet;
};