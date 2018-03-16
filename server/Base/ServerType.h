#pragma once
#include "MainType.h"

//#define SERVER_TYPE_MAIN 0

// ping消息
#define SVR_SUB_PING 1

// 服务器注册
#define SVR_SUB_SERVER_REGISTER 2

// 服务器注册返回
#define SVR_SUB_SERVER_REGISTER_RET 3

//服务器负载信息
#define SVR_SUB_SERVER_LOADINFO 4

//服务器负载更新
#define SVR_SUB_UPDATE_LOAD 5

// 新的玩家
#define SVR_SUB_ADD_PLAYER_TO_CENTER 6

// 玩家断开
#define SVR_SUB_DEL_CLIENT 8

// 通知game玩家的账号信息
#define SVR_SUB_CLIENT_ACCOUNT 9

// 玩家数据
#define SVR_SUB_PLAYERDATA 10

// 切换线路
#define  SVR_SUB_CHANGELINE 11

// 切换线路返回
#define  SVR_SUB_CHANGELINE_RET 12

// 踢下client
#define SVR_SUB_KICKCLIENT 13

// 名称检查
#define SVR_SUB_NAMECHECK 14

// 名称检查返回
#define SVR_SUB_NAMECHECK_RET 15

// 服务器log
#define SVR_SUB_LOG 16