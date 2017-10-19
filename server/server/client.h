/*
*
*	Client对象
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
		enum_remove_delay_time = 5000,		//延时移除 ms
	};

	//设置连接时间
	void SetConnectTime(int64 currenttime) { m_connecttime = currenttime; }
	//设置ping的时间
	void SetPingTime(int64 currenttime) { m_pingtime = currenttime; }
	//是否超时
	bool bOverTime(int64 currenttime,int clientovertime);
	void SetPlayer(playerobj * player) { m_player = player; }
	playerobj *GetPlayer() { return m_player; }
public:
	
	//设置client的socket
	void SetSocket(lxnet::Socketer *socket);
	//获取client的socket
	lxnet::Socketer *GetSocket() { return m_socket; }
	//发送消息给客户端
	void SendMsg(Msg *pMsg);
	//获取消息
	Msg *GetMsg();
	//设置IP（socket里面获取）
	void SetIP(const char *ip) { memcpy(m_ip, ip, strlen(ip)); }
	//获取IP
	char *GetIP() { return m_ip; }
private:

	int64 m_connecttime;	//连接时间
	int64 m_pingtime;		//ping的时间

	char m_ip[128];
	//该client的socket
	lxnet::Socketer *m_socket;
	playerobj *m_player;
};