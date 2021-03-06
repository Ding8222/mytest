﻿/*
* 玩家操作
* Copyright (C) ddl
* 2018
*/

#pragma once
#include "Player.h"
#include "msgbase.h"
#include "ServerLog.h"
#include "Utilities.h"

void Operate(CPlayer *pPlayer, Msg *pMsg);

void DoClientMsg(CPlayer *pPlayer, Msg *pMsg);

void DoLoginMsg(CPlayer *pPlayer, Msg *pMsg);

void DoTeamMsg(CPlayer *pPlayer, Msg *pMsg);
