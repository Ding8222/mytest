#include "Player.h"
#include "serverlog.h"
#include "msgbase.h"
#include "Utilities.h"
#include "Timer.h"
#include "GameGatewayMgr.h"
#include "CSVLoad.h"
#include "InstanceMgr.h"
#include "SceneMgr.h"

#include "ServerType.h"
#include "ServerMsg.pb.h"

CPlayer::CPlayer() :CBaseObj(EOT_PLAYER)
{
	m_Instance = 0;
	m_InstanceBaseId = 0;
}

CPlayer::~CPlayer()
{

}

void CPlayer::Run()
{
	CBaseObj::Run();

	if (CTimer::GetTime() > m_LastSaveTime + 30 * 60)
	{
		// 保存数据
		SaveData();
	}
}

void CPlayer::SendMsgToMe(Msg &pMsg, bool bRef)
{
	if (bRef)
	{
		SendRefMsg(pMsg);
	}
	else
	{
		static msgtail tail;
		tail.id = GetClientID();
		GameGatewayMgr.SendMsg(GetGateInfo(), pMsg, &tail, sizeof(tail));
	}
}

void CPlayer::Die()
{

}

// 上线
bool CPlayer::OnLine()
{
	// 存在副本id的时候，尝试进入副本
	if (m_InstanceBaseId > 0)
	{
		do
		{
			// 存在实例id的时候,尝试进入已经存在的副本
			if (m_Instance > 0)
			{
				CInstance* instance = InstanceMgr.FindInstance(m_Instance);
				if (instance)
				{
					// 副本存在的时候，直接进去
					if (!InstanceMgr.EnterInstance(this, instance))
						return false;
					else
						break;
				}
			}

			// 不存在的时候创建
			m_Instance = InstanceMgr.AddInstance(m_InstanceBaseId);
			if (m_Instance == 0)
				return false;

			// 进入副本
			if (!InstanceMgr.EnterInstance(this, m_Instance))
				return false;
		} while (false);
	}
	else
	{
		CScene *_pScene = SceneMgr.FindScene(GetMapID());
		if (!_pScene)
		{
			RunStateError("没有找到玩家：%s要登陆的地图：%d", GetName(), GetMapID());
			return false;
		}

		if (!_pScene->AddObj(this))
		{
			RunStateError("添加玩家：%s到地图：%d，失败！", GetName(), GetMapID());
			return false;
		}
	}

	return true;
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