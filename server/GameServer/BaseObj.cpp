#include <assert.h>
#include "BaseObj.h"
#include "ServerLog.h"
#include "Utilities.h"
#include "msgbase.h"
#include "GameGatewayMgr.h"

#include "ClientType.h"
#include "ClientMsg.pb.h"

CBaseObj::CBaseObj(int8 nObjType):m_ObjType(nObjType)
{
	// 根据obj类型设置aoi模式
	if (m_ObjType == EOT_PLAYER)
		SetAoiMode("wm");
	else
		SetAoiMode("m");

	m_ObjSex = 0;
	m_ObjJob = 0;
	m_ObjLevel = 0;
	m_WaitRemoveTime = 0;
	m_DieTime = 0;
	memset(m_ObjName, 0, MAX_NAME_LEN);
}

CBaseObj::~CBaseObj()
{

}

void CBaseObj::Run()
{
	AoiRun();
	StatusRun();
	FightRun();
}
std::unordered_map<serverinfo *, std::list<int32>> CBaseObj::gateinfo;
void CBaseObj::SendRefMsg(Msg &pMsg)
{
	gateinfo.clear();
	static bool bNeeSendMsg = false;
	static msgtail tail;
	std::unordered_map<uint32, CBaseObj *> *playerlist = GetAoiList();
	std::unordered_map<uint32, CBaseObj *>::iterator iter = playerlist->begin();
	for (; iter != playerlist->end(); ++iter)
	{
		if (iter->second->IsPlayer())
		{
			CPlayer * p = (CPlayer *)iter->second;
			if (FuncUti::isValidCret(p))
			{
				auto iter = gateinfo.find(p->GetGateInfo());
				if (iter != gateinfo.end())
				{
					std::list<int32> &temp = iter->second;
					temp.push_back(p->GetClientID());
					bNeeSendMsg = true;
				}
				else
				{
					std::list<int32> temp;
					temp.push_back(p->GetClientID());
					gateinfo.insert(std::make_pair(p->GetGateInfo(), temp));
					bNeeSendMsg = true;
				}
			}
		}
	}

	static bool bIsPlayer = false;
	bIsPlayer = IsPlayer();
	if (bNeeSendMsg || bIsPlayer)
	{
		if (bIsPlayer)
		{
			CPlayer *p = (CPlayer *)this;
			auto iter = gateinfo.find(p->GetGateInfo());
			if (iter != gateinfo.end())
			{
				std::list<int32> &temp = iter->second;
				temp.push_back(p->GetClientID());
			}
			else
			{
				std::list<int32> temp;
				temp.push_back(p->GetClientID());
				gateinfo.insert(std::make_pair(p->GetGateInfo(), temp));
			}
		}

		bNeeSendMsg = false;
		MessagePack pkmain;
		pkmain.PushInt32(pMsg.GetLength());
		pkmain.PushBlock(&pMsg, pMsg.GetLength());
		int32 pkmainlen = pkmain.GetLength();

		for (auto &i : gateinfo)
		{
			tail.id = 0;
			std::list<int32> &temp = i.second;
			for (auto &j : temp)
			{
				pkmain.PushInt32(j);
				--tail.id;
			}
			GameGatewayMgr.SendMsg(i.first, pkmain, &tail, sizeof(tail));
			pkmain.SetLength(pkmainlen);
			pkmain.m_index = pkmainlen - (int32)sizeof(Msg);
		}
	}
}

void CBaseObj::UpdataObjInfo(CBaseObj *obj)
{
	netData::UpdataObjInfo sendMsg;
	
	if (obj)
	{
		// 将obj的信息发送给我
		sendMsg.set_name(obj->GetName());
		sendMsg.set_ntempid(obj->GetTempID());
		sendMsg.set_x(obj->GetNowPosX());
		sendMsg.set_y(obj->GetNowPosY());
		sendMsg.set_z(obj->GetNowPosZ());

		FuncUti::SendPBNoLoop(ToPlayer(), sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_UPTATE_OBJINFO);
	}
	else
	{
		// 广播我的信息
		sendMsg.set_name(GetName());
		sendMsg.set_ntempid(GetTempID());
		sendMsg.set_x(GetNowPosX());
		sendMsg.set_y(GetNowPosY());
		sendMsg.set_z(GetNowPosZ());

		FuncUti::SendPBNoLoop(ToPlayer(), sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_UPTATE_OBJINFO, true);
	}
}

void CBaseObj::DelObjFromView(uint32 tempid)
{
	netData::DelObjFromView sendMsg;
	sendMsg.set_ntempid(tempid);

	FuncUti::SendPBNoLoop(ToPlayer(), sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_DELOBJ_FROM_VIEW);
}