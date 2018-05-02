/*
* 玩家数据保存、加载
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "platform_config.h"

// 数据长度(int32) 数据编号(int8) 数据版本(int8) 数据个数(int32)
static int32 s_Data_Base_Size = sizeof(int32) + sizeof(int8) + sizeof(int8) + sizeof(int32);

// 保存数据之前的检查
#define SAVE_DATA_CHECK()\
int32 maxsize = retlen;\
retlen = 0;\
if (maxsize < s_Data_Base_Size) return false

// 设置数据编号
#define  SET_DATA_NUM(num)\
len = sizeof(int32);\
(*((int8*)(dest + len)) = (num))

// 设置数据版本号
#define  SET_DATA_VER(ver)\
len += sizeof(int8);\
(*((int8*)(dest + len)) = (ver))

// 设置数据个数
#define SET_DATA_COUNT(num)\
len += sizeof(int8);\
(*((int32*)(dest + len)) = (num));\
len += sizeof(int32)

// 设置数据长度
#define SET_DATA_LEN \
*((int32*)dest) = (len);\
retlen = (len)

#define GET_DATA_CHECK \
if (retlen == 0) return true;\
maxsize = retlen;\
retlen = 0;\
if (maxsize < s_Data_Base_Size) return false;\
if (maxsize < *((int32*)dest)) return false

#define GET_DATA_NUM \
*((int8*)(dest + sizeof(int32)))

#define GET_DATA_VER \
*((int8*)(dest + sizeof(int32) + sizeof(int8)))

#define GET_DATA_COUNT \
*((int32*)(dest + sizeof(int32) + sizeof(int8) + sizeof(int8)))

// 背包数据
#define PACKAGE_DATA_NUM 1
#define PACKAGE_DATA_VER 1
