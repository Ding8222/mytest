#include "lxnet\base\crosslib.h"
#include "Guid.h"

extern uint64 get_time();

CGuid::CGuid()
{
	nEpoch = 0;
	nMachine = 0;
	nSequence = 0;
}

CGuid::~CGuid()
{

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
