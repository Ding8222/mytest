#pragma once
#include <string>
#include "platform_config.h"

class blockbuf;
class CStringPool
{
public:
	CStringPool();
	~CStringPool();

	static bool InitPools();

	static void DestroyPools();

	//重置
	void Reset();

	//装入一个字符串
	bool PushMsg(std::string &str);

	//获取一个字符串
	const char *GetMsg();

private:
	void CheckBlockNull();
	int32 getdata(char *buf, int32 len);
	void Destroy();

private:

	blockbuf *m_head;
	blockbuf *m_currentforpush;
	blockbuf *m_currentforget;
};

CStringPool *string_create();

void string_release(void *self);
