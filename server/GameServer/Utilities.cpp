#include <string>
#include <windows.h>
#include "Utilities.h"
#include "GameGatewayMgr.h"

namespace FuncUti
{
	char *UnicodeToUTF_8(const std::wstring &str)
	{
		int u8Len = WideCharToMultiByte(CP_UTF8, NULL, str.c_str(), str.length(), NULL, 0, NULL, NULL);
		char* szU8 = new  char[u8Len + 1];
		WideCharToMultiByte(CP_UTF8, NULL, str.c_str(), str.length(), szU8, u8Len, NULL, NULL);
		szU8[u8Len] = '\0';
		return szU8;
	}

	wchar_t *Utf_8ToUnicode(const char* szU8)
	{
		int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), NULL, 0);
		wchar_t* wszString = new wchar_t[wcsLen + 1];
		::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), wszString, wcsLen);
		wszString[wcsLen] = '\0';
		return wszString;
	}

	bool isValidCret(CBaseObj *obj)
	{
		if (obj && !obj->IsWaitRemove())
			return true;
		return false;
	}

	void SendPBNoLoop(CPlayer *player, google::protobuf::Message &pMsg, int maintype, int subtype, bool bRef)
	{
		if (!isValidCret(player))
			return;

		serverinfo *gate = player->GetGateInfo();
		assert(gate);
		if (!gate)
			return;

		msgtail tail;
		if (!bRef)
		{
			tail.id = player->GetClientID();
			CGameGatewayMgr::Instance().SendMsg(gate, pMsg, maintype, subtype, &tail, sizeof(tail));
		}
		else
		{
			MessagePack pkmain;
			MessagePack pk;
#if _DEBUG
			assert(pk.Pack(&pMsg, maintype, subtype));
#else
			if (pk.Pack(&pMsg, maintype, subtype))
#endif
			{
				pkmain.PushInt32(pk.GetLength());
				pkmain.PushBlock(&pk, pk.GetLength());

				std::unordered_map<uint32, CBaseObj *> *playerlist = player->GetAoiList();
				std::unordered_map<uint32, CBaseObj *>::iterator iter = playerlist->begin();
				for (; iter != playerlist->end(); ++iter)
				{
					if (iter->second->IsPlayer())
					{
						CPlayer * p = (CPlayer *)iter->second;
						if (isValidCret(p))
						{
							pkmain.PushInt32(p->GetClientID());
							--tail.id;
						}
					}
				}

				// 添加自己
				pkmain.PushInt32(player->GetClientID());
				--tail.id;
				CGameGatewayMgr::Instance().SendMsg(gate, pkmain, &tail, sizeof(tail));
			}
		}
	}

	void SendMsg(CPlayer *player, Msg &pMsg, bool bRef)
	{
		if (!isValidCret(player))
			return;

		serverinfo *gate = player->GetGateInfo();
		assert(gate);
		if (!gate)
			return;

		msgtail tail;
		if (bRef)
		{
			tail.id = player->GetClientID();
			CGameGatewayMgr::Instance().SendMsg(gate, pMsg, &tail, sizeof(tail));
		}
		else
		{
			MessagePack pkmain;
			pkmain.PushInt32(pMsg.GetLength());
			pkmain.PushBlock(&pMsg, pMsg.GetLength());

			std::unordered_map<uint32, CBaseObj *> *playerlist = player->GetAoiList();
			std::unordered_map<uint32, CBaseObj *>::iterator iter = playerlist->begin();
			for (; iter != playerlist->end(); ++iter)
			{
				if (iter->second->IsPlayer())
				{
					CPlayer * p = (CPlayer *)iter->second;
					if (isValidCret(p))
					{
						pkmain.PushInt32(p->GetClientID());
						--tail.id;
					}
				}
			}

			// 添加自己
			pkmain.PushInt32(player->GetClientID());
			--tail.id;
			CGameGatewayMgr::Instance().SendMsg(gate, pMsg, &tail, sizeof(tail));
		}
	}
}