/*
* 背包
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>
#include "platform_config.h"

class CItem;
class CPackage
{
public:
	CPackage();
	~CPackage();

	bool Init();
	void Run();
	void Destroy();
	bool Save(char *dest, int &retlen);
	bool Load(const char *dest, int &retlen);

	// 添加道具
	bool AddItem(int32 itemid, int32 count);
	bool AddItem(CItem *item);
	// 删除道具
	bool DelItem(int32 itemid, int32 count);
	bool DelItem(int64 guid, int32 count);
	// 查找道具
	CItem *GetItem(int64 guid);

private:
	// 背包最大数量
	int32 m_MaxCount;
	std::unordered_map<int64, CItem *> m_Package;
};
