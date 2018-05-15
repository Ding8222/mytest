/*
* 全局定义
* 
*/
#pragma once

#define MAX_NAME_LEN 64
#define MAX_SECRET_LEN 16

#define GATE_ID_MAX 10					//网关最大id
#define CLIENT_ID_MAX 800000			//客户端最大id
#define CLIENT_ID_DELAY_TIME 300000		//释放延时时间
#define SCENE_ID_MAX 160000				//地图场景中对象的最大id

#define _CHECK_PARSE_(x,y) if(!((MessagePack *)x)->UnPack(y)) return

namespace ServerEnum {
	enum ServerType
	{
		EST_UNKNOW = 0,
		EST_CENTER,		//中心
		EST_GAME,		//逻辑
		EST_GATE,		//网关
		EST_LOGIN,		//登陆
		EST_DB,			//数据库
		EST_LOG,		//日志
		EST_NAME,		//名称检查

		EST_END,
	};
}


namespace MapEnum {
	enum MapType
	{
		EMT_NORMAL = 0,		//普通场景地图
		EMT_INSTANCE,		//副本地图

		EST_END,
	};
}

#pragma pack(push, 1)

#pragma pack(pop)