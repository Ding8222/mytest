/*
* Client管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <list>
#include <vector>
#include "lxnet.h"

class CClient;
struct idmgr;
class CClientMgr
{
public:

	CClientMgr();
	~CClientMgr();

	bool Init(int maxclientnum, int listenport, int clientovertime, int recvdatalimit, int senddatalimit);
	void Run();
	void EndRun();
	void Destroy();

	void GetCurrentInfo(char *buf, size_t buflen);
	void SendMsgToServerByID(Msg *pMsg, int clientid);
	CClient *FindClient(int clientid);

	//设置某个客户端认证成功
	void SetClientAuthSucceed(int clientid);

	//延时关闭某个客户端
	void DelayCloseClient(int clientid);
	void ReleaseAllClient();
	void AsLogicServerDisconnect(int logicserverid);

	//获取当前连接的客户端数量
	int GetClientConnectNum() { return (int)m_ClientList.size(); }

	virtual int64 OnNewClient();
	virtual void OnClientDisconnect(CClient *cl);
	virtual void ProcessClientMsg(CClient *cl) = 0;

	// 发送消息
	void SendMsg(CClient *cl, google::protobuf::Message &pMsg, int maintype, int subtype);
	void SendMsg(CClient *cl, Msg *pMsg);
	void SendMsg(int64 clientid, google::protobuf::Message &pMsg, int maintype, int subtype);
	void SendMsg(int64 clientid, Msg *pMsg);
private:
	void StopListen();
	bool TestAndListen();
	void AcceptNewClient();
	void ProcessAllClient();
	void CheckAndRemove();
	void ReleaseClientAndID(CClient *cl);
	bool InitIdMgrAndClientSet();
private:
	int m_MaxClientNum;
	int m_ListenPort;
	int m_OverTime;

	lxnet::Listener *m_Listen;

	std::list<CClient *> m_ClientList;
	std::list<CClient *> m_WaitRemove;

	std::vector<CClient *> m_ClientSet;
	idmgr *m_IDPool;

	int m_RecvDataLimit;
	int m_SendDataLimit;
};
