/*
*
*	Client���ӹ�����
*
*/

#pragma once

static const int s_backlog = 128;

class clientmgr
{
public:
	clientmgr();
	~clientmgr();

	static clientmgr &Instance()
	{
		static clientmgr m;
		return m;
	}

	//��ʼ��--ָ�������˿ںʹ���listener
	bool init(int port,int clientovertime);
	//�ͷ���Դ
	void release();
	//������������
	void testandlisten();
	//���ͽ����µ�����
	void acceptnewclient();
	//�������е�client����
	void processallclient();
	//client��msg����
	void processclientmsg(client *cl);
	//ÿ֡����
	void run();
	void endrun();

public:
	//����Ϣ���͸�����client����
	void SendMsgToAll(Msg *pMsg,client *cl = NULL);

private:
	//client���ӶϿ��Ĵ���
	void OnClientDisconnect(client *cl);

private:
	//�����˿�
	int m_listen_port;
	//�Ƿ���ҪListen()
	bool m_needlisten;
	//��ʱʱ��
	int m_clientovertime;

	//listener
	lxnet::Listener *m_listen;

	//���е�client����
	std::set<client *> m_client_list;
};