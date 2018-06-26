/*
* 道具
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <string>
#include "lxnet\base\platform_config.h"
#include "CSVLoad.h"

#pragma pack(push,1)
struct stItem
{
	stItem()
	{
		nBaseID = 0;
		nCount = 0;
		nGuid = 0;
	}

	// 道具基本ID
	int32 nBaseID;
	// 道具数量
	int32 nCount;
	// 道具唯一ID
	int64 nGuid;
};

#pragma pack(pop)

class CItem;
class CItemCreator
{
public:
	CItemCreator(){}
	~CItemCreator(){}

	static CItem *CreateItem();
	static void ReleaseItem(CItem *item);
private:
};

class CItem
{
public:
	CItem();
	~CItem();

	bool Init(int32 baseid, int32 count);
	void Run();
	void Destroy();

public:
	int32 GetBaseID() { return m_ItemInfo.nBaseID; }
	int64 GetGuid() { return m_ItemInfo.nGuid; }
	int32 GetCount() { return m_ItemInfo.nCount; }
	// 改变道具数量
	bool CountChange(int32 count);
	// 获取道具数据
	stItem *GetItemInfo() { return &m_ItemInfo; }
	// 加载道具数据
	bool LoadItem(stItem *iteminfo);
private:
	// 道具实例数据
	stItem m_ItemInfo;
	// 道具模板数据指针
	CSVData::stItem *m_pItemDB;
};
