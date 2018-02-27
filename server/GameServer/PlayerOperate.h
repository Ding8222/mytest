#pragma once

#include "Player.h"
class CPlayerOperate
{
public:
	CPlayerOperate();
	~CPlayerOperate();

	static void SetPlayer(CPlayer *player) { m_pPlayer = player; }
	static void Operate(Msg *pMsg, int64 clientid);
private:
	static CPlayer * m_pPlayer;
};