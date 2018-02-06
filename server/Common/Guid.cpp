#include "crosslib.h"
#include "Guid.h"
#if defined(__GUNC__)
#include <sys/time.h>
#include <unistd.h>
#define EPOCHFILETIME 11644473600000000ULL
#else
#include <windows.h>
#include <time.h>
#define EPOCHFILETIME 11644473600000000Ui64
#endif

CGuid::CGuid()
{
	nEpoch = 0;
	nMachine = 0;
	nSequence = 0;
}

CGuid::~CGuid()
{

}

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

void CGuid::SetEpoch(uint64 epoch)
{
	nEpoch = epoch;
}

void CGuid::SetMachine(int32 machine)
{
	nMachine = machine;
}

int64 CGuid::Generate()
{
	int64 value = 0;
	uint64 time = get_time() - nEpoch;

	// 保留后39位时间
	value = time << 22;

	// 中间12位是机器ID
	value |= (nMachine & 0x3FF) << 12;

	// 最后12位是sequenceID
	value |= nSequence++ & 0xFFF;
	if (nSequence == 0x1000)
	{
		nSequence = 0;
	}

	return value;
}
