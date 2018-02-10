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

	bool IsAuth() { return m_isAuth; }
	bool IsHandShake() { return m_isHandShake; }
	void SetHandShake(bool handshake) { m_isHandShake = handshake; }

	void SendMsg(google::protobuf::Message &pMsg, int maintype, int subtype, void *adddata = nullptr, size_t addsize = 0);

	void OnConnectDisconnect();

	void ChangeConnect(const char *ip, int port, int id, bool bauth);

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
	std::string GetSecret() { return sSecret; }
	void SetAccount(std::string name) { sAccount = name; }
	std::string GetAccount() { return sAccount; }
private:
	bool m_isHandShake;
	bool m_isAuth;

	CryptoPP::SecByteBlock sClientKey;
	CryptoPP::SecByteBlock sServerKey;
	std::string sSecret;
	std::string sAccount;
};