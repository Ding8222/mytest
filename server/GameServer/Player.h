/*
* 玩家对象
* Copyright (C) ddl
* 2018
*/

#pragma once
#include<unordered_map>
#include "BaseObj.h"
#include "Package.h"
#include "Team.h"
#include "serverinfo.h"
#include "google/protobuf/Message.h"

class serverinfo;
class CClient;
class scene;
struct Msg;
class CPlayer :public CBaseObj, public CTeam
{
public:
	CPlayer();
	~CPlayer();

	virtual CPlayer *GetPlayer() { return this; }
	virtual void Run();
	virtual void SendMsgToMe(Msg &pMsg, bool bRef = false);
	virtual void Die();
	// 加载数据
	bool LoadData(Msg *pMsg);
	bool SaveData();
	// 保存数据,pMsg不为nullptr的时候，打包数据至pMsg
	bool SaveData(google::protobuf::Message *pMsg);
	// 打包数据
	bool PackData();
	// 解析数据
	bool UnPackData(const char *data,int32 len);
	// 下线
	void OffLine();
	
private:
	// 背包
	CPackage m_Package;
public:
	void SetGateInfo(serverinfo * info) { m_GateInfo = info; }
	serverinfo *GetGateInfo() { return m_GateInfo; }
	int32 GetGateID() { if (m_GateInfo) return m_GateInfo->GetServerID(); return 0; }
	void SetClientID(int32 id) { m_ClientID = id; }
	int32 GetClientID() { return m_ClientID; }
	void SetAccount(const std::string &account) { m_Account = std::move(account); }
	std::string GetAccount() { return m_Account; }
	void SetGuid(int64 id) { m_Guid = id; }
	int64 GetGuid() { return m_Guid; }
	void SetCreateTime(int64 time) { m_CreateTime = time; }
	int64 GetCreateTime() { return m_CreateTime; }
	void SetLoginTime(int64 time) { m_LoginTime = time; }
	int64 GetLoginTime() { return m_LoginTime; }
private:
	serverinfo * m_GateInfo;
	int32 m_ClientID;
	std::string m_Account;
	int64 m_Guid;
	int64 m_CreateTime;
	int64 m_LoginTime;
	int64 m_LastSaveTime;
	bool m_LoadDataSucc;
};