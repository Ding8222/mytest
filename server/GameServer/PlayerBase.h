/*
* 玩家的一些基础属性
* Copyright (C) ddl
* 2019
*/

#pragma once
#include "lxnet\base\platform_config.h"
#include "serverinfo.h"

class CPlayerBase
{
public:
	CPlayerBase();
	~CPlayerBase();

public:
	void SetGateInfo(serverinfo * info) { m_GateInfo = info; }
	serverinfo *GetGateInfo() { return m_GateInfo; }
	int32 GetGateID() { if (m_GateInfo) return m_GateInfo->GetServerID(); return 0; }
	void SetClientID(int32 id) { m_ClientID = id; }
	int32 GetClientID() { return m_ClientID; }
	void SetGameID(int64 id) { m_GameID = id; }
	int64 GetGameID() { return m_GameID; }
	void SetAccount(const std::string &account) { m_Account = std::move(account); }
	std::string GetAccount() { return m_Account; }
	void SetGuid(int64 id) { m_Guid = id; }
	int64 GetGuid() { return m_Guid; }
	void SetCreateTime(int64 time) { m_CreateTime = time; }
	int64 GetCreateTime() { return m_CreateTime; }
	void SetLoginTime(int64 time) { m_LoginTime = time; }
	int64 GetLoginTime() { return m_LoginTime; }

protected:
	// 网关对象
	serverinfo * m_GateInfo;
	// 网关中的ID
	int32 m_ClientID;
	// Game中的ID和网关中的ID组合出的唯一ID
	int64 m_GameID;
	// 账号
	std::string m_Account;
	// 唯一ID
	int64 m_Guid;
	// 创建时间
	int64 m_CreateTime;
	// 登录时间
	int64 m_LoginTime;
	// 上次数据保存时间
	int64 m_LastSaveTime;
	// 加载数据是否成功
	bool m_LoadDataSucc;
};