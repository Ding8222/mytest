#pragma once

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

// 新的client连接
#define SVR_SUB_NEW_CLIENT 6

// 新的client连接返回
#define SVR_SUB_NEW_CLIENT_RET 7

// client断开
#define SVR_SUB_DEL_CLIENT 8

// 通知game玩家的token
#define SVR_SUB_CLIENT_TOKEN 9

// 通知game加载数据
#define SVR_SUB_LOAD_PLAYERDATA 10

// 切换线路
#define  SVR_SUB_CHANGELINE 11

// 切换线路返回
#define  SVR_SUB_CHANGELINE_RET 12