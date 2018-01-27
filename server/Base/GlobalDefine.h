/*
* 全局定义
* 
*/
#pragma once
#include "platform_config.h"
#include <string>
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

#pragma pack(push, 1)
struct ClientSvr
{
	ClientSvr()
	{
		ServerType = 0;
		ServerID = 0;
		ClientID = 0;
		Token.clear();
		Secret.clear();
	}

	ClientSvr(int servertype, int serverid, int64 clientid)
	{
		ServerType = servertype;
		ServerID = serverid;
		ClientID = clientid;
		Token.clear();
		Secret.clear();
	}
	int ServerType;
	int ServerID;
	int64 ClientID;

	std::string Token;
	std::string Secret;
};
#pragma pack(pop)