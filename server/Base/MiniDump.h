/*
* MiniDump
* Copyright (C) ddl
* 2018
*/
#pragma once

#ifdef _WIN32

class CMiniDump
{
public:
	static bool Begin(void);
	static bool End(void);
};

#endif