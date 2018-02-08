/*
* 全局定义
* 
*/
#pragma once
#include "platform_config.h"
#include <string>
#define MAX_IP_LEN 128
#define MAX_NAME_LEN 64

#define CLIENT_ID_MAX 800000			//客户端最大id
#define CLIENT_ID_DELAY_TIME 300000		//释放延时时间
#define SCENE_ID_MAX 160000				//地图场景中对象的最大id
#define INSTANCE_ID_MAX 2000			//副本数量
#define INSTANCE_ID_DELAY_TIME 300000	//释放延时时间

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