/*
* Client管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <list>
#include <vector>
#include "lxnet.h"
#include <google/protobuf/message.h>
#include "lxnet\base\platform_config.h"

class CClient;
struct idmgr;
class CClientMgr
{
public:

	CClientMgr();
	~CClientMgr();

	bool Init(int32 maxclientnum, int32 listenport, int32 clientovertime, int32 recvdatalimit, int32 senddatalimit);
	void Run();
	void EndRun();
	virtual void Destroy();

	void GetCurrentInfo(char *buf, size_t buflen);
	CClient *FindClient(int32 clientid);

	//设置某个客户端认证成功
	void SetClientAuthSucceed(int32 clientid);

	//延时关闭某个客户端
	void DelayCloseClient(int32 clientid);
	virtual void ReleaseAllClient();
	void AsLogicServerDisconnect(int32 logicserverid);

	//获取当前连接的客户端数量
	int32 GetClientConnectNum() { return (int32)m_ClientList.size(); }

	virtual int32 OnNewClient();
	virtual void OnClientDisconnect(CClient *cl);
	virtual void ProcessClientMsg(CClient *cl) = 0;

	// 发送消息
	void SendMsg(CClient *cl, google::protobuf::Message &pMsg, int32 maintype, int32 subtype);
	void SendMsg(CClient *cl, Msg *pMsg);
	void SendMsg(int32 clientid, google::protobuf::Message &pMsg, int32 maintype, int32 subtype);
	void SendMsg(int32 clientid, Msg *pMsg);

	CClient *FindClientByClientID(int32 clientid);
private:
	void StopListen();
	bool TestAndListen();
	void AcceptNewClient();
	void ProcessAllClient();
	void CheckAndRemove();
	void ReleaseClientAndID(CClient *cl);
	bool InitIdMgrAndClientSet();
private:
	int32 m_MaxClientNum;
	int32 m_ListenPort;
	int32 m_OverTime;

	lxnet::Listener *m_Listen;

	std::list<CClient *> m_ClientList;
	std::list<CClient *> m_WaitRemove;

	std::vector<CClient *> m_ClientSet;
	idmgr *m_IDPool;

	int32 m_RecvDataLimit;
	int32 m_SendDataLimit;
};
