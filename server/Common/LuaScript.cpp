#include "LuaScript.h"
#include "lua.hpp"
#include "ServerLog.h"
#include "crosslib.h"

CLuaScript::CLuaScript()
{
	m_Init = false;
	m_Lua = nullptr;
}

CLuaScript::~CLuaScript()
{
	Destroy();
}

bool CLuaScript::Init()
{
	if (!m_Lua)
	{
		m_Lua = new sol::state;
		m_Lua->open_libraries();
		return Register();
	}
	return false;
}

void CLuaScript::Destroy()
{
	if (m_Lua)
	{
		delete m_Lua;
		m_Lua = nullptr;
		m_Init = false;
	}
}

void CLuaScript::Run()
{

}

static void SetScriptInitFlag(bool flag)
{
	LuaScript.SetInitFlag(flag);
}

bool CLuaScript::Register()
{
	m_Lua->set_function("SetScriptInitFlag", SetScriptInitFlag);
	return true;
}

bool CLuaScript::DoInitScriptFile(const char *filename)
{
	if (!m_Lua->do_file(filename).valid())
	{
		RunStateError("加载%s失败！", filename);
		return false;
	}

	if (!m_Init)
	{
		RunStateError("初始化%s失败！", filename);
		return false;
	}
	
	RunStateLog("加载%s成功！", filename);
	return true;
}