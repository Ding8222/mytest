#include "PlayerOperate.h"
#include "Utilities.h"
#include "msgbase.h"

#include "DoClientMsg.h"
#include "DoLoginMsg.h"

#include "ServerType.h"

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