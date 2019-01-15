/*
* 玩家对象
* Copyright (C) ddl
* 2018
*/

#pragma once
#include<unordered_map>
#include "BaseObj.h"
#include "PlayerBase.h"
#include "Package.h"
#include "PlayerGameLevel.h"
#include "Team.h"
#include "google/protobuf/Message.h"

class CPlayer :public CPlayerBase, public CBaseObj, public CTeam
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
	bool UnPackData(const char *data, int32 len);
	// 上线
	bool OnLine();
	// 下线
	void OffLine();
	
private:
	// 背包
	CPackage m_Package;
	// 关卡
	CPlayerGameLevel m_GameLevel;
public:
private:
	// 副本实例ID
	int32 m_Instance;
	// 副本模板Id
	int32 m_InstanceBaseId;
};