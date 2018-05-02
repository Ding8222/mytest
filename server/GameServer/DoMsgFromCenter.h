#pragma once
#include "connector.h"
#include "msgbase.h"

extern int64 g_currenttime;

void DoServerMsgFromCenter(connector *con, Msg *pMsg, msgtail *tl);

void DoTeamMsgFromCenter(connector *con, Msg *pMsg, msgtail *tl);