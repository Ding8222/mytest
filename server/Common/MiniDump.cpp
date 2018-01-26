#ifdef _WIN32
#include <stdio.h>
#include <string>
#include <windows.h>
#include <dbghelp.h>
#include <tchar.h> 
#pragma   comment(   lib,   "DbgHelp.lib"   )

#include "MiniDump.h"

#define _CRT_STDIO_ISO_WIDE_SPECIFIERS

extern void SendEMailToInternet(std::string str, char* addr = "ding8222@vip.qq.com");

typedef bool(WINAPI *MINIDUMPWRITEDUMP)( // Callback 
	HANDLE hProcess,
	DWORD dwPid,
	HANDLE hFile,
	MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

LPTOP_LEVEL_EXCEPTION_FILTER PreviousExceptionFilter = NULL;

LONG WINAPI UnHandledExceptionFilter(struct _EXCEPTION_POINTERS *exceptionInfo)
{
	HMODULE	DllHandle = NULL;

	DllHandle = LoadLibrary(_T("DBGHELP.DLL"));

	if (DllHandle)
	{
		MINIDUMPWRITEDUMP Dump = (MINIDUMPWRITEDUMP)GetProcAddress(DllHandle, "MiniDumpWriteDump");

		if (Dump)
		{
			TCHAR		DumpPath[MAX_PATH] = { 0, };
			SYSTEMTIME	SystemTime;

			GetLocalTime(&SystemTime);

			//获取进程名
			TCHAR szFileFullPath[256];
			::GetModuleFileName(NULL, static_cast<TCHAR *>(szFileFullPath), 256);
			std::wstring str(szFileFullPath);
			int pos = str.find_last_of(_T("\\"));
			std::wstring str2 = &str[pos + 1];
			_snwprintf_s(DumpPath, MAX_PATH, MAX_PATH, _T("CrashDumps\\%ws %d-%d-%d %d_%d_%d_%d.dmp"),
				str2.c_str(),
				SystemTime.wYear,
				SystemTime.wMonth,
				SystemTime.wDay,
				SystemTime.wHour,
				SystemTime.wMinute,
				SystemTime.wSecond,
				SystemTime.wMilliseconds
				);

			HANDLE FileHandle = CreateFile(
				DumpPath,
				GENERIC_WRITE,
				FILE_SHARE_WRITE,
				NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
			if (FileHandle == INVALID_HANDLE_VALUE)
			{
				CreateDirectory(_T("CrashDumps"), 0);

				FileHandle = CreateFile(
					DumpPath,
					GENERIC_WRITE,
					FILE_SHARE_WRITE,
					NULL, CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
			}

			if ((FileHandle != NULL) && (FileHandle != INVALID_HANDLE_VALUE))
			{
				_MINIDUMP_EXCEPTION_INFORMATION MiniDumpExceptionInfo;

				MiniDumpExceptionInfo.ThreadId = GetCurrentThreadId();
				MiniDumpExceptionInfo.ExceptionPointers = exceptionInfo;
				MiniDumpExceptionInfo.ClientPointers = NULL;

				//设置包含内容
				MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(
					MiniDumpWithDataSegs|
					MiniDumpWithProcessThreadData|
					MiniDumpWithHandleData |
					MiniDumpWithPrivateReadWriteMemory|
					MiniDumpWithUnloadedModules|
					MiniDumpWithFullMemoryInfo |
					MiniDumpWithFullMemory |
					MiniDumpWithThreadInfo );

				bool Success = Dump(
					GetCurrentProcess(),
					GetCurrentProcessId(),
					FileHandle,
					mdt,
					&MiniDumpExceptionInfo,
					NULL,
					NULL);

				if (Success)
				{
					INT iLength = 0;
					iLength = WideCharToMultiByte(CP_UTF8, 0, str2.c_str(), -1, NULL, 0, NULL, NULL);
					char TempStr[256];
					WideCharToMultiByte(CP_UTF8, 0, str2.c_str(), -1, TempStr, iLength, NULL, NULL);
					SendEMailToInternet(TempStr);
					SendEMailToInternet(TempStr,"13545162711@139.com");
					CloseHandle(FileHandle);

					return EXCEPTION_EXECUTE_HANDLER;
				}
			}

			CloseHandle(FileHandle);
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

bool CMiniDump::Begin(void)
{
	SetErrorMode(SEM_FAILCRITICALERRORS);

	PreviousExceptionFilter = SetUnhandledExceptionFilter(UnHandledExceptionFilter);

	return true;
}

bool CMiniDump::End(void)
{
	SetUnhandledExceptionFilter(PreviousExceptionFilter);

	return true;
}

#endif