#include <assert.h>
#include "BaseObj.h"
#include "ServerLog.h"
#include "Utilities.h"

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

void CBaseObj::UpdataObjInfo(CBaseObj *obj)
{
	netData::UpdataObjInfo sendMsg;
	
	if (obj)
	{
		// 将obj的信息发送给我
		sendMsg.set_name(obj->GetName());
		sendMsg.set_ntempid(obj->GetTempID());
		sendMsg.set_x(obj->GetPosX());
		sendMsg.set_y(obj->GetPosY());
		sendMsg.set_z(obj->GetPosZ());

		FuncUti::SendPBNoLoop(ToPlayer(), sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_UPTATE_OBJINFO);
	}
	else
	{
		// 广播我的信息
		sendMsg.set_name(GetName());
		sendMsg.set_ntempid(GetTempID());
		sendMsg.set_x(GetPosX());
		sendMsg.set_y(GetPosY());
		sendMsg.set_z(GetPosZ());

		FuncUti::SendPBNoLoop(ToPlayer(), sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_UPTATE_OBJINFO, true);
	}
}

void CBaseObj::DelObjFromView(uint32 tempid)
{
	netData::DelObjFromView sendMsg;
	sendMsg.set_ntempid(tempid);

	FuncUti::SendPBNoLoop(ToPlayer(), sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_DELOBJ_FROM_VIEW);
}