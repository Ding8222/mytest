#pragma once
#include "serverinfo.h"
#include "msgbase.h"

extern int64 g_currenttime;

void DoServerMsg(serverinfo *info, Msg *pMsg, msgtail *tl);

void DoLoginMsg(serverinfo *info, Msg *pMsg, msgtail *tl);

void DoTeamMsg(serverinfo *info, Msg *pMsg, msgtail *tl);