#include "Player.h"
#include "scene.h"
#include "scenemgr.h"
#include "serverlog.h"
#include "msgbase.h"
#include "Utilities.h"
#include "Timer.h"
#include "GameGatewayMgr.h"

#include "ServerType.h"
#include "ServerMsg.pb.h"

CPlayer::CPlayer():CBaseObj(EOT_PLAYER)
{
	m_GateInfo = nullptr;
	m_ClientID = 0;
	m_Guid = 0;
	m_CreateTime = 0;
	m_LoginTime = 0;
	m_LastSaveTime = 0;
	m_LoadDataSucc = false;
}

CPlayer::~CPlayer()
{

}

void CPlayer::Run()
{
	CBaseObj::Run();

	if (CTimer::GetTime() > m_LastSaveTime + 5 * 30)
	{
		// 5分钟保存一次数据
		SaveData();
	}
}

void CPlayer::SendMsgToMe(Msg &pMsg, bool bRef)
{
	msgtail tail;
	tail.id = GetClientID();
	CGameGatewayMgr::Instance().SendMsg(GetGateInfo(), pMsg, &tail, sizeof(tail));

	if (bRef)
	{
		std::unordered_map<uint32, CBaseObj *> *playerlist = GetAoiList();
		std::unordered_map<uint32, CBaseObj *>::iterator iter = playerlist->begin();
		for (; iter != playerlist->end(); ++iter)
		{
			if (iter->second->IsPlayer())
			{
				CPlayer * p = (CPlayer *)iter->second;
				if (FuncUti::isValidCret(p))
				{
					tail.id = p->GetClientID();
					CGameGatewayMgr::Instance().SendMsg(p->GetGateInfo(), pMsg, &tail, sizeof(tail));
				}
			}
		}
	}
}

void CPlayer::Die()
{

}

// 下线
void CPlayer::OffLine()
{
	if (IsWaitRemove())
		return;

	LeaveScene();

	SaveData();
	//放在最后
	SetWaitRemove();
}