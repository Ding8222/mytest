
#ifdef _WIN32
#include <windows.h>
#include <iostream>
#include "CtrlHandler.h"
#include "log.h"

CCtrlHandler::CCtrlHandler()
{
	f_Fun = nullptr;
}
CCtrlHandler::~CCtrlHandler()
{
	f_Fun = nullptr;
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:
		log_error("Ctrl-C event");
		CCtrlHandler::Instance().Destroy();

		return(TRUE);

		// CTRL-CLOSE: confirm that the user wants to exit. 
	case CTRL_CLOSE_EVENT:
		log_error("Ctrl-Close event");
		CCtrlHandler::Instance().Destroy();
		return(TRUE);

		// Pass other signals to the next handler. 
	case CTRL_BREAK_EVENT:
		log_error("Ctrl-Break event");
		return FALSE;

	case CTRL_LOGOFF_EVENT:
		log_error("Ctrl-Logoff event");
		return FALSE;

	case CTRL_SHUTDOWN_EVENT:
		log_error("Ctrl-Shutdown event");
		return FALSE;

	default:
		return FALSE;
	}
}

bool CCtrlHandler::Init(Callback *cb)
{
	f_Fun = cb;
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
	{
		log_error("初始化CtrlHandler失败！");
		return false;
	}
	return true;
}

void CCtrlHandler::Destroy()
{
	(f_Fun)();
}

#endif