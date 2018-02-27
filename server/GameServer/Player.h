#pragma once
#include<unordered_map>
#include "BaseObj.h"

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
	// 下线
	void OffLine();

public:
	void SetGateInfo(serverinfo * info) { m_GateInfo = info; }
	serverinfo * GetGateInfo() { return m_GateInfo; }
	void SetClientID(int id) { m_ClientID = id; }
	int GetClientID() { return m_ClientID; }
private:
	serverinfo * m_GateInfo;
	int m_ClientID;
};