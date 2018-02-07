/*
* 全局定义
* 
*/
#pragma once
#include "platform_config.h"
#include <string>
#define MAX_IP_LEN 128
#define MAX_NAME_LEN 64

namespace ServerEnum {
	enum ServerType
	{
		EST_UNKNOW = 0,
		EST_CENTER,		//中心
		EST_GAME,		//逻辑
		EST_GATE,		//网关
		EST_LOGIN,		//登陆
		EST_DB,			//数据库

		EST_END,
	};
}

#define _CHECK_PARSE_(x,y) if(!((MessagePack *)x)->UnPack(y)) return

#pragma pack(push, 1)

#pragma pack(pop)