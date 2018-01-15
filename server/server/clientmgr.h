/*
*
*	Client链接管理类
*
*/
#pragma once
static const int s_backlog = 128;

class CClient;

class CClientMgr
{
public:
	CClientMgr();
	~CClientMgr();

	static CClientMgr &Instance()
	{
		static CClientMgr m;
		return m;
	}

	//初始化--指定监听端口和创建listener
	bool Init(int port,int clientovertime);
	//释放资源
	void Release();
	//尝试启动监听
	void TestAndListen();
	//检测和接受新的连接
	void AcceptNewClient();
	//处理所有的client连接
	void ProcessAllClient();
	//每帧调用
	void Run();
	void EndRun();

public:
	//将消息发送给所有client连接
	void SendMsgToAll(Msg *pMsg, CClient *cl = NULL);

private:
	//client连接断开的处理
	void OnClientDisconnect(CClient *cl);

private:
	//监听端口
	int m_ListenPort;
	//是否需要Listen()
	bool m_NeedListen;
	//超时时间
	int m_ClientOverTime;
	//listener
	lxnet::Listener *m_Listen;
	//所有的client连接
	std::set<CClient *> m_ClientList;
};