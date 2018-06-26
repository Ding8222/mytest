#pragma once
#include <string>
#include "lxnet.h"
#include "lxnet\base\platform_config.h"

class CBackCommand
{
public:
	enum
	{
		enum_servername_maxlen = 512,
		enum_sock_buf_data_limit = 524288,	//512kb
	};
	CBackCommand();
	~CBackCommand();

	bool Init(void(*f_dofunc) (lxnet::Socketer *sock), int32 port, int32 pingtime, const char *name);
	void Run(int64 currenttime);
	const char *GetName() { return m_Name.c_str(); }
private:
	void Destroy();
private:
	int64 m_LastSend;
	std::string m_ConnectIP;
	int32 m_PingTime;
	std::string m_Name;

	void(*m_DoFunction)(lxnet::Socketer *sock);

	lxnet::Listener *m_Listen;
	lxnet::Socketer *m_Sock;
};