/*
*
*	Client���ӹ�����
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

	//��ʼ��--ָ�������˿ںʹ���listener
	bool Init(int port,int clientovertime);
	//�ͷ���Դ
	void Release();
	//������������
	void TestAndListen();
	//���ͽ����µ�����
	void AcceptNewClient();
	//�������е�client����
	void ProcessAllClient();
	//ÿ֡����
	void Run();
	void EndRun();

public:
	//����Ϣ���͸�����client����
	void SendMsgToAll(Msg *pMsg, CClient *cl = NULL);

private:
	//client���ӶϿ��Ĵ���
	void OnClientDisconnect(CClient *cl);

private:
	//�����˿�
	int m_ListenPort;
	//�Ƿ���ҪListen()
	bool m_NeedListen;
	//��ʱʱ��
	int m_ClientOverTime;
	//listener
	lxnet::Listener *m_Listen;
	//���е�client����
	std::set<CClient *> m_ClientList;
};