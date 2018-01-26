/*
* Windwos信号捕捉
* Copyright (C) ddl
* 2018
*/
#ifdef _WIN32
#pragma once

typedef void (Callback)();

class CCtrlHandler
{
public:
	CCtrlHandler();
	~CCtrlHandler();

	static CCtrlHandler &Instance()
	{
		static CCtrlHandler m;
		return m;
	}

	bool Init(Callback *cb);
	virtual void Destroy();

private:
	Callback *f_Fun;
};

#endif