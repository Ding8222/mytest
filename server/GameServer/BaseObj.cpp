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

void CBaseObj::SendRefMsg(Msg &pMsg)
{
	assert(m_GateIDMax < gateinfo_count);
	for (int i = m_GateIDMin; i <= m_GateIDMax; ++i)
	{
		gateinfo[i]->Reset();
	}

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
				int32 gateid = p->GetGateID();
				if (gateid >= 0 && gateid < gateinfo_count)
				{
					if (gateid > m_GateIDMax)
					{
						m_GateIDMax = gateid;
					}
					else if (gateid < m_GateIDMin)
					{
						m_GateIDMin = gateid;
					}

					if (gateinfo[gateid]->gate == nullptr)
					{
						gateinfo[gateid]->gate = p->GetGateInfo();
					}
					gateinfo[gateid]->nClientId[gateinfo[gateid]->nCount] = p->GetClientID();
					gateinfo[gateid]->nCount += 1;
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
			int32 gateid = p->GetGateID();
			if (gateid >= 0 && gateid < gateinfo_count)
			{
				if (gateid > m_GateIDMax)
				{
					m_GateIDMax = gateid;
				}
				else if (gateid < m_GateIDMin)
				{
					m_GateIDMin = gateid;
				}

				if (gateinfo[gateid]->gate == nullptr)
				{
					gateinfo[gateid]->gate = p->GetGateInfo();
				}

				gateinfo[gateid]->nClientId[gateinfo[gateid]->nCount] = p->GetClientID();
				gateinfo[gateid]->nCount += 1;
			}
		}

		bNeeSendMsg = false;
		MessagePack pkmain;
		pkmain.PushInt32(pMsg.GetLength());
		pkmain.PushBlock(&pMsg, pMsg.GetLength());
		int32 pkmainlen = pkmain.GetLength();

		for (int i = m_GateIDMin; i < m_GateIDMax; ++i)
		{
			if (gateinfo[i]->nCount > 0)
			{
				tail.id = 0;
				for (int j = 0; j < gateinfo[i]->nCount; ++j)
				{
					pkmain.PushInt32(gateinfo[i]->nClientId[j]);
					--tail.id;
				}
				GameGatewayMgr.SendMsg(gateinfo[i]->gate, pkmain, &tail, sizeof(tail));
				pkmain.SetLength(pkmainlen);
				pkmain.m_index = pkmainlen - (int32)sizeof(Msg);
			}
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

std::vector<stGateInfo *> CBaseObj::gateinfo;
int32 CBaseObj::m_GateIDMin;
int32 CBaseObj::m_GateIDMax;

bool CBaseObj::InitGateInfo()
{
	gateinfo.resize(gateinfo_count);
	for (int i = 0; i < gateinfo_count; ++i)
	{
		stGateInfo *newgateinfo = new stGateInfo;
		newgateinfo->Clean();
		newgateinfo->gate = nullptr;
		gateinfo[i] = newgateinfo;
	}
	m_GateIDMin = 3000;
	m_GateIDMax = gateinfo_count - 1;
	return true;
}

void CBaseObj::ReleaseGateInfo()
{
	for (size_t i = 0; i < gateinfo.size(); ++i)
	{
		delete gateinfo[i];
	}
	gateinfo.clear();
}