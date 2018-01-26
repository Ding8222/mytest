/*
* 全局定义
* 
*/
#pragma once
#define MAX_IP_LEN 128

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

#define _CHECK_PARSE_(x,y) if(!((MessagePack *)x)->UnPack(y)) return;