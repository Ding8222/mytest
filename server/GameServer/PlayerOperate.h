﻿#pragma once
#include "Player.h"

void Operate(CPlayer *pPlayer, Msg *pMsg);

void DoLoginMsg(CPlayer *pPlayer, Msg *pMsg);

void DoClientMsg(CPlayer *pPlayer, Msg *pMsg);