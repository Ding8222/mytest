/*
* 机器人连接器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <string>
#include "connector.h"
#include "osrng.h"
class CRobot :public connector
{
public:
	CRobot();
	~CRobot();

	// 目标地址、端口、远程服务器id、本地id、本地类型、ping时间、超时重试时间
	bool Init(const char *ip, int port, int id);
	
	bool IsReady() { return m_IsReady; }
	void SetReady(bool ready) { m_IsReady = ready; }

	void SendMsg(google::protobuf::Message &pMsg, int maintype, int subtype, void *adddata = nullptr, size_t addsize = 0);

	void OnConnectDisconnect();

	void Destroy();
private:
	// 注册前处理的消息
	void ProcessRegister(connector *);
	// 注册后处理的消息
	void ProcessMsg(connector *);

public:
	void SetClientKey(CryptoPP::SecByteBlock key) { sClientKey = key; }
	void SetServerKey(CryptoPP::SecByteBlock key) { sServerKey = key; }
	void SetSecret(std::string key) { sSecret = key; }
private:
	bool m_IsReady;

	char m_IP[MAX_IP_LEN];
	int m_Port;
	int m_ID;

	CryptoPP::SecByteBlock sClientKey;
	CryptoPP::SecByteBlock sServerKey;
	std::string sSecret;
};