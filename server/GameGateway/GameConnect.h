﻿/*
* 逻辑服务器连接
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ConnectMgr.h"

class CGameConnect : public CConnectMgr
{
public:
	CGameConnect();
	~CGameConnect();

	static CGameConnect &Instance()
	{
		static CGameConnect m;
		return m;
	}

	bool Init();

	void ServerRegisterSucc(int id, const char *ip, int port);
	// 连接断开的处理
	void ConnectDisconnect(connector *);
	// 处理远程服务器发来的消息
	void ProcessMsg(connector *_con);

	void AddNewClientSvrToken(ClientSvr *cl);
	ClientSvr *FindClientSvr(std::string t);
	bool AddNewClientSvrID(std::string token, int servertype, int serverid, int64 clientid);
	ClientSvr *FindClientSvr(int64 clientid);
private:

	// token,ClientSvr
	std::unordered_map < std::string, ClientSvr *> m_ClientSvrToken;
	// clientid,ClientSvr
	std::unordered_map < int64, ClientSvr *> m_ClientSvrID;
};