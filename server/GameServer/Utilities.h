﻿#pragma once
#include "Player.h"
#include "google/protobuf/Message.h"
#include "GameCenterConnect.h"
#include "Config.h"

namespace FuncUti
{
	char *UnicodeToUTF_8(const std::wstring &str);
	wchar_t *Utf_8ToUnicode(const char* szU8);
	inline bool isValidCret(CBaseObj *player);
	// 发送PB消息给玩家
	// 不允许在循环中调用!!!
	// 需要循环发送的使用SendMsg
	void SendPBNoLoop(CBaseObj *player, google::protobuf::Message &pMsg, int maintype, int subtype, bool bRef = false);

	inline void SendMsgToCenter(CPlayer *player, google::protobuf::Message &pMsg, int maintype, int subtype)
	{
		CGameCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), pMsg, maintype, subtype, player->GetGuid());
	}
	inline void SendMsgToCenter(CPlayer *player, Msg &pMsg, bool bRef)
	{
		CGameCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), pMsg, player->GetGuid());
	}
}