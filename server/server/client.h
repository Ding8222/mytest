/*
*
*	Client����
*
*/

#pragma once

class CPlayer;

class CClient
{
public:
	CClient();
	~CClient();

	enum
	{
		enum_remove_delay_time = 5000,		//��ʱ�Ƴ� ms
	};

	// ��������ʱ��
	void SetConnectTime(int64 currenttime) { m_ConnectTime = currenttime; }
	// ����ping��ʱ��
	void SetPingTime(int64 currenttime) { m_PingTime = currenttime; }
	// �Ƿ�ʱ
	bool bOverTime(int64 currenttime,int clientovertime);
	// ���õ�ǰClient��������Player
	void SetPlayer(CPlayer * player) { m_Player = player; }
	// ��ȡ��ǰClient��������Player
	CPlayer *GetPlayer() { return m_Player; }
public:
	
	// ����CClient��socket
	void SetSocket(lxnet::Socketer *socket);
	// ��ȡCClient��socket
	lxnet::Socketer *GetSocket() { return m_Socket; }
	// ������Ϣ���ͻ���
	void SendMsg(Msg *pMsg);
	// ��ȡ��Ϣ
	Msg *GetMsg();
	// ������Ϣ
	void ProcessMsg();
	// ����IP��socket�����ȡ��
	void SetIP(std::string ip) { m_Ip = ip; }
	// ��ȡIP
	std::string GetIP() { return m_Ip; }
private:

	int64 m_ConnectTime;	//����ʱ��
	int64 m_PingTime;		//ping��ʱ��

	std::string m_Ip;
	//��CClient��socket
	lxnet::Socketer *m_Socket;
	CPlayer *m_Player;
};