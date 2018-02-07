#include <string>
#include <windows.h>
#include "Utilities.h"

namespace FuncUti
{
	char *UnicodeToUTF_8(std::wstring str)
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

	bool isValidCret(CBaseObj *player)
	{
		if (player && !player->IsWaitRemove())
			return true;
		return false;
	}
}