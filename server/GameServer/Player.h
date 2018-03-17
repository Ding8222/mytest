#pragma once
#include<unordered_map>
#include "BaseObj.h"
#include "serverinfo.h"
#include "google/protobuf/Message.h"

class serverinfo;
class CClient;
class scene;
struct Msg;
class CPlayer :public CBaseObj
{
public:
	CPlayer();
	~CPlayer();

	void Run();
	// 加载数据
	bool LoadData(Msg *pMsg);
	// 保存数据
	bool SaveData();
	// 打包数据
	bool PackData(google::protobuf::Message *pMsg);
	// 下线
	void OffLine();

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
	void SetData(const char *data, int32 size) { memcpy_s(m_Data, 256, data, size); }
	char *GetData() { return m_Data; }
private:
	serverinfo * m_GateInfo;
	int32 m_ClientID;
	std::string m_Account;
	int64 m_Guid;
	int64 m_CreateTime;
	int64 m_LoginTime;
	int64 m_LastSaveTime;
	char m_Data[256];
};