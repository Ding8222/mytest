#pragma once
#include "sol.hpp"

#define LuaScript CLuaScript::Instance()
class CLuaScript
{
public:
	CLuaScript();
	~CLuaScript();
	static CLuaScript &Instance()
	{
		static CLuaScript m;
		return m;
	}

	bool Init();
	void Destroy();
	void Run();

	bool Register();
	bool DoInitScriptFile(const char *filename);

	sol::state *GetState() { return m_Lua; }

	void SetInitFlag(bool flag) { m_Init = flag; }
private:
	bool m_Init;
	sol::state *m_Lua;
};