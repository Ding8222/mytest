#include "crosslib.h"
#include <Timer.h>

#if defined(__GUNC__)
#include <sys/time.h>
#include <unistd.h>
#define EPOCHFILETIME 11644473600000000ULL
#else
#include <windows.h>
#include <time.h>
#define EPOCHFILETIME 11644473600000000Ui64
#endif

uint64 get_time()
{
#ifdef __GUNC__
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64 time = tv.tv_usec;
	time /= 1000;
	time += ((uint64)tv.tv_sec * 1000);
	return time;
#else
	FILETIME filetime;
	uint64 time = 0;
	GetSystemTimeAsFileTime(&filetime);

	time |= filetime.dwHighDateTime;
	time <<= 32;
	time |= filetime.dwLowDateTime;

	time /= 10;
	time -= EPOCHFILETIME;
	return time / 1000;
#endif
}

int64 CTimer::m_Time = time(nullptr);
uint64 CTimer::m_Time64 = get_time();

CTimer::CTimer()
{

}

CTimer::~CTimer()
{

}

void CTimer::UpdateTime()
{
	m_Time64 = get_time();
	m_Time = time(nullptr);
}