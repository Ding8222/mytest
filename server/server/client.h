/*
*
*	Client对象
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
		enum_remove_delay_time = 5000,		//延时移除 ms
	};

	// 设置连接时间
	void SetConnectTime(int64 currenttime) { m_ConnectTime = currenttime; }
	// 设置ping的时间
	void SetPingTime(int64 currenttime) { m_PingTime = currenttime; }
	// 是否超时
	bool bOverTime(int64 currenttime,int clientovertime);
	// 设置当前Client链接所属Player
	void SetPlayer(CPlayer * player) { m_Player = player; }
	// 获取当前Client链接所属Player
	CPlayer *GetPlayer() { return m_Player; }
public:
	
	// 设置CClient的socket
	void SetSocket(lxnet::Socketer *socket);
	// 获取CClient的socket
	lxnet::Socketer *GetSocket() { return m_Socket; }
	// 发送消息给客户端
	void SendMsg(Msg *pMsg);
	// 获取消息
	Msg *GetMsg();
	// 处理消息
	void ProcessMsg();
	// 设置IP（socket里面获取）
	void SetIP(std::string ip) { m_Ip = ip; }
	// 获取IP
	std::string GetIP() { return m_Ip; }
private:

	int64 m_ConnectTime;	//连接时间
	int64 m_PingTime;		//ping的时间

	std::string m_Ip;
	//该CClient的socket
	lxnet::Socketer *m_Socket;
	CPlayer *m_Player;
};