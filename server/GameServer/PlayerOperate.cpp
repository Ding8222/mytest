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

CPlayer * CPlayerOperate::m_pPlayer = NULL;
CPlayerOperate::CPlayerOperate()
{

}
CPlayerOperate::~CPlayerOperate()
{

}

void CPlayerOperate::Operate(Msg *pMsg, int64 clientid)
{
	if (!pMsg)
		return;

	if (!FuncUti::isValidCret(m_pPlayer))
		return;

	switch (pMsg->GetMainType())
	{
	case LOGIN_TYPE_MAIN:
	{
		// 		switch (pMsg->GetSubType())
		// 		{
		// 		default:
		// 		{
		// 		}
		// 		}
		// 		break;
	}
	case CLIENT_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case CLIENT_SUB_MOVE:
		{
			netData::PlayerMove msg;
			_CHECK_PARSE_(pMsg, msg);

			netData::PlayerMoveRet sendMsg;
			sendMsg.set_ntempid(m_pPlayer->GetTempID());
			if (m_pPlayer->MoveTo(msg.x(), msg.y(), msg.z()))
			{
				sendMsg.set_x(msg.x());
				sendMsg.set_y(msg.y());
				sendMsg.set_z(msg.z());
			}
			else
			{
				float _Pos[EPP_MAX] = { 0 };
				m_pPlayer->GetNowPos(_Pos[EPP_X], _Pos[EPP_Y], _Pos[EPP_Z]);
				sendMsg.set_x(_Pos[EPP_X]);
				sendMsg.set_y(_Pos[EPP_Y]);
				sendMsg.set_z(_Pos[EPP_Z]);
			}
			FuncUti::SendPBNoLoop(m_pPlayer,sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_MOVE_RET,true);
			break;
		}
		default:
		{
		}
		}
		break;
	}
	default:
		break;
	}
}