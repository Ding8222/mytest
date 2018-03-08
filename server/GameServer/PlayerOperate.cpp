#include "PlayerOperate.h"
#include "Utilities.h"
#include "msgbase.h"
#include "GameGatewayMgr.h"

#include "MainType.h"
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
			float _Pos[EPP_MAX] = { 0 };
			pPlayer->GetNowPos(_Pos[EPP_X], _Pos[EPP_Y], _Pos[EPP_Z]);
			sendMsg.set_x(_Pos[EPP_X]);
			sendMsg.set_y(_Pos[EPP_Y]);
			sendMsg.set_z(_Pos[EPP_Z]);
		}
		FuncUti::SendPBNoLoop(pPlayer, sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_MOVE_RET, true);
		break;
	}
	case CLIENT_SUB_CHANGEMAP:
	{
		netData::ChangeMap msg;
		_CHECK_PARSE_(pMsg, msg);


		break;
	}
	}
}
