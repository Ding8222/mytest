/*
* RandomPool
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "crosslib.h"
#include <random>

class CRandomPool
{
public:
	CRandomPool();
	~CRandomPool();

	static uint32 GetOne();
	// [0,num)
	static uint32 GetOneLess(uint32 num);
	// [min,max]
	static uint32 GetOneBetween(uint32 min, uint32 max);
private:

	static std::random_device rd;
	static std::default_random_engine gen;
	static std::uniform_int_distribution<int> dis;
};