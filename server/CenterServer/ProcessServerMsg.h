#pragma once
#include "serverinfo.h"
#include "msgbase.h"
#include "DoMsg.h"

void ProcessLoginMsg(serverinfo *info, Msg *pMsg, msgtail *tl);

void ProcessGateMsg(serverinfo *info, Msg *pMsg, msgtail *tl);

void ProcessGameMsg(serverinfo *info, Msg *pMsg, msgtail *tl);

void ProcessDBMsg(serverinfo *info, Msg *pMsg, msgtail *tl);