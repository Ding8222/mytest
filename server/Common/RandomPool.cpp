#include "RandomPool.h"

CRandomPool::CRandomPool()
{

}

CRandomPool::~CRandomPool()
{

}

std::random_device CRandomPool::rd;
std::default_random_engine CRandomPool::gen = std::default_random_engine(CRandomPool::rd());
std::uniform_int_distribution<int> CRandomPool::dis(1,9999999);

uint32 CRandomPool::GetOne()
{
	return dis(gen);
}

uint32 CRandomPool::GetOneLess(uint32 num)
{
	if (num == 0)
		return 0;

	return dis(gen) % num;
}

uint32 CRandomPool::GetOneBetween(uint32 min, uint32 max)
{
	if (min > max)
		min = max;

	return dis(gen) % (max - min + 1) + min;
}
