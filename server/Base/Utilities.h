#pragma once

namespace FuncUti
{
	char *UnicodeToUTF_8(std::wstring str);
	wchar_t *Utf_8ToUnicode(const char* szU8);
}