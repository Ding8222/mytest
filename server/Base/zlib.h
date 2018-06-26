/*
* zlib封装
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "lxnet\base\platform_config.h"

int32 Compress(const char* inBuf, uint32 inSize, const char* outBuf, int32 &outSize);
int32 UnCompress(const char* inBuf, uint32 inSize, const char* outBuf, int32 &outSize);