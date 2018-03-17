﻿#include "PlayerOperate.h"
#include "Utilities.h"
#include "msgbase.h"
#include "GameGatewayMgr.h"
#include "SceneMgr.h"
#include "MapConfig.h"
#include "Scene.h"
#include "serverlog.h"
#include "PlayerMgr.h"

#include "ServerType.h"
#include "ClientType.h"
#include "LoginType.h"
#include "ServerMsg.pb.h"
#include "Login.pb.h"
#include "ClientMsg.pb.h"

void Operate(CPlayer *pPlayer, Msg *pMsg)
{
	if (!pMsg)
		return;

	if (!FuncUti::isValidCret(pPlayer))
		return;

	switch (pMsg->GetMainType())
	{
	case LOGIN_TYPE_MAIN:
	{
		DoLoginMsg(pPlayer, pMsg);
 		break;
	}
	case CLIENT_TYPE_MAIN:
	{
		DoClientMsg(pPlayer, pMsg);
		break;
	}
	}
}

void DoLoginMsg(CPlayer *pPlayer, Msg *pMsg)
{

}

void DoClientMsg(CPlayer *pPlayer, Msg *pMsg)
{
	switch (pMsg->GetSubType())
	{
	case CLIENT_SUB_MOVE:
	{
		netData::PlayerMove msg;
		_CHECK_PARSE_(pMsg, msg);

		netData::PlayerMoveRet sendMsg;
		sendMsg.set_ntempid(pPlayer->GetTempID());
		if (pPlayer->MoveTo(msg.x(), msg.y(), msg.z()))
		{
			sendMsg.set_x(msg.x());
			sendMsg.set_y(msg.y());
			sendMsg.set_z(msg.z());
		}
		else
		{
			float _Pos[EOP_MAX] = { 0 };
			pPlayer->GetNowPos(_Pos[EOP_X], _Pos[EOP_Y], _Pos[EOP_Z]);
			sendMsg.set_x(_Pos[EOP_X]);
			sendMsg.set_y(_Pos[EOP_Y]);
			sendMsg.set_z(_Pos[EOP_Z]);
		}
		FuncUti::SendPBNoLoop(pPlayer, sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_MOVE_RET, true);
		break;
	}
	case CLIENT_SUB_CHANGEMAP:
	{
		netData::ChangeMap msg;
		_CHECK_PARSE_(pMsg, msg);

		netData::ChangeMapRet SendMsg;
		SendMsg.set_bchangeip(false);
		int32 nMapID = msg.nmapid();
		if (nMapID != pPlayer->GetMapID())
		{
			CScene *scene = CSceneMgr::Instance().FindScene(nMapID);
			if (scene)
			{
				// 本线路存在的地图
				if (pPlayer->LeaveScene())
				{
					if (scene->AddObj(pPlayer))
					{
						SendMsg.set_ncode(netData::ChangeMapRet::EC_SUCC);
					}
				}
				else
				{
					RunStateError("玩家[%s][%d]离开地图[%d]失败，下线!", 
						pPlayer->GetName(), pPlayer->GetTempID(), 
						pPlayer->GetScene() == nullptr ? -1 : pPlayer->GetScene()->GetMapID()
					);
					pPlayer->OffLine();
					SendMsg.set_ncode(netData::ChangeMapRet::EC_LEAVEMAP);
				}
			}
			else
			{
				// 本线路不存在的地图
				if (CMapConfig::Instance().isValidMapID(nMapID))
				{
					// 在其他线路
					// 向Center请求转移进入其他线路地图
					svrData::ChangeLine ChangeLineMsg;
					ChangeLineMsg.set_nlineid(0);
					ChangeLineMsg.set_nmapid(nMapID);
					svrData::LoadPlayerData *pData = ChangeLineMsg.mutable_data();
					if (pData)
					{
						if (pPlayer->PackData(pData))
						{
							pData->set_nmapid(nMapID);
	#ifdef _DEBUG
							RunStateError("换线!玩家[%s]离开地图[%d]，目标地图[%d]",
								pPlayer->GetName(), pPlayer->GetScene() == nullptr ? -1 : pPlayer->GetScene()->GetMapID(),
								nMapID
							);
	#endif
							FuncUti::SendMsgToCenter(pPlayer, ChangeLineMsg, SERVER_TYPE_MAIN, SVR_SUB_CHANGELINE);
							pPlayer->OffLine();
							return;
						}
						else
						{
							RunStateError("换线！玩家：%s 打包数据失败！", pPlayer->GetName());
							SendMsg.set_ncode(netData::ChangeMapRet::EC_PACKDATA);
						}
					}
				}
				else
				{
					RunStateError("玩家：%s 要去的地图：%d 不存在！", pPlayer->GetName(), nMapID);
					SendMsg.set_ncode(netData::ChangeMapRet::EC_MAP);
				}
				FuncUti::SendPBNoLoop(pPlayer, SendMsg, SERVER_TYPE_MAIN, SVR_SUB_CHANGELINE);
			}
		}
		break;
	}
	}
}
