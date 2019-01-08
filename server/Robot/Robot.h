/*
* 机器人连接器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <string>
#include "connector.h"
class CRobot :public connector
{
public:
	CRobot();
	~CRobot();

	bool IsAuth() { return m_isAuth; }
	bool IsHandShake() { return m_isHandShake; }
	void SetHandShake(bool handshake) { m_isHandShake = handshake; }

	void SendMsg(google::protobuf::Message &pMsg, int8 maintype, int8 subtype, void *adddata = nullptr, size_t addsize = 0);

	void OnConnectDisconnect();

	void ChangeConnect(const char *ip, int port, int id, bool bauth);

	void Destroy();
private:
	// 注册前处理的消息
	void ProcessRegister(connector *);
	// 注册后处理的消息
	void ProcessMsg(connector *);

public:
	void SetClientKey(std::string key) { sClientKey = std::move(key); }
	void SetServerKey(std::string key) { sServerKey = std::move(key); }
	void SetSecret(std::string key) { sSecret = std::move(key); }
	std::string GetSecret() { return sSecret; }
	void SetAccount(std::string name) { sAccount = std::move(name); }
	std::string GetAccount() { return sAccount; }
	void SetTempID(int32 id) { m_TempID = id; }
	int32 GetTempID() { return m_TempID; }
	void SetChangeMap() { bChangeMap = true; }
	bool GetChangeMap() { return bChangeMap; }
private:
	bool m_isHandShake;
	bool m_isAuth;

	std::string sClientKey;
	std::string sServerKey;
	std::string sSecret;
	std::string sAccount;
	int32 m_TempID;
	bool bChangeMap;
};