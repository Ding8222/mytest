#pragma once
#include "Player.h"
#include "google/protobuf/Message.h"

namespace FuncUti
{
	char *UnicodeToUTF_8(std::wstring str);
	wchar_t *Utf_8ToUnicode(const char* szU8);
	bool isValidCret(CBaseObj *player);
	// 发送PB消息给玩家
	// 不允许在循环中调用!!!
	// 需要循环发送的使用SendMsg
	void SendPBNoLoop(CPlayer *player, google::protobuf::Message &pMsg, int maintype, int subtype, bool bRef = false);
	// 需要循环调用发送给玩家的，先把PB消息打包成Msg之后，调用该消息循环发送
	void SendMsg(CPlayer *player, Msg &pMsg, bool bRef = false);
	void SendMsgToCenter(CPlayer *player, google::protobuf::Message &pMsg, int maintype, int subtype);
	void SendMsgToCenter(CPlayer *player, Msg &pMsg, bool bRef);
}