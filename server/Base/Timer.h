/*
* Timer
* Copyright (C) ddl
* 2018
*/
#pragma once

class CTimer
{
public:
	CTimer();
	~CTimer();
	
	static int64 GetTime() { return m_Time; }
	static uint64 GetTime64() { return m_Time64; }
	static void UpdateTime();
private:

	static int64 m_Time;
	static uint64 m_Time64;
};
