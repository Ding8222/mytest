#pragma once

// twitter snowflake算法
// 64       63--------------24---------12---------0
// 符号位   |     39位时间   |12位机器码|12位自增码|
class CGuid
{
public:
	CGuid();
	~CGuid();

	static CGuid &Instance()
	{
		static CGuid m;
		return m;
	}

	void SetEpoch(uint64 epoch);
	void SetMachine(int32 machine);
	int64 Generate();

private:
	uint64 nEpoch;
	int32 nMachine;
	int32 nSequence;
};
