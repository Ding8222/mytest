/*
*
*	Client����
*
*/

#pragma once

class client
{
public:
	client();
	~client();

	enum
	{
		enum_remove_delay_time = 5000,		//��ʱ�Ƴ� ms
	};

	//��������ʱ��
	void SetConnectTime(int64 currenttime) { m_connecttime = currenttime; }
	//����ping��ʱ��
	void SetPingTime(int64 currenttime) { m_pingtime = currenttime; }
	//�Ƿ�ʱ
	bool bOverTime(int64 currenttime,int clientovertime);

public:
	
	//����client��socket
	void SetSocket(lxnet::Socketer *socket);
	//��ȡclient��socket
	lxnet::Socketer *GetSocket() { return m_socket; }
	//������Ϣ���ͻ���
	void SendMsg(Msg *pMsg);
	//��ȡ��Ϣ
	Msg *GetMsg();
	//����IP��socket�����ȡ��
	void SetIP(const char *ip) { memcpy(m_ip, ip, strlen(ip)); }
	//��ȡIP
	char *GetIP() { return m_ip; }
private:

	int64 m_connecttime;	//����ʱ��
	int64 m_pingtime;		//ping��ʱ��

	char m_ip[128];
	//��client��socket
	lxnet::Socketer *m_socket;
};