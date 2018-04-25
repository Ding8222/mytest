#include "Item.h"
#include "Guid.h"
#include "Timer.h"
#include "objectpool.h"

#define ITEM_MAX 100000

static objectpool<CItem> &ItemPool()
{
	static objectpool<CItem> m(ITEM_MAX, "CItem pools");
	return m;
}

static CItem *item_create()
{
	CItem *self = ItemPool().GetObject();
	if (!self)
	{
		RunStateError("创建 CItem 失败!");
		return NULL;
	}
	new(self) CItem();
	return self;
}

static void item_release(CItem *self)
{
	if (!self)
		return;
	self->~CItem();
	ItemPool().FreeObject(self);
}

CItem *CItemCreator::CreateItem()
{
	CItem *item = item_create();
	if (!item)
	{
		RunStateError("创建CItem失败!");
		return nullptr;
	}

	return item;
}

void CItemCreator::ReleaseItem(CItem *item)
{
	item_release(item);
}


CItem::CItem()
{
	m_pItemDB = nullptr;
}

CItem::~CItem()
{
	Destroy();
}

bool CItem::Init(int32 baseid, int32 count)
{
	CSVData::stItem *iteminfo = CSVData::CItemDB::FindById(baseid);
	if (iteminfo)
	{
		if (count > 0 && count <= iteminfo->nMaxCount)
		{
			m_pItemDB = iteminfo;
			m_ItemInfo.nBaseID = iteminfo->nItemID;
			m_ItemInfo.nCount = count;
			m_ItemInfo.nGuid = CGuid::Instance().Generate();
			return true;
		}
	}
	return false;
}

void CItem::Run()
{

}

void CItem::Destroy()
{

}

bool CItem::CountChange(int32 count)
{
	if (count > 0)
	{
		if (m_pItemDB->nMaxCount - m_ItemInfo.nCount >= count)
		{
			m_ItemInfo.nCount += count;
			return true;
		}
	}
	else
	{
		if (m_ItemInfo.nCount >= abs(count))
		{
			m_ItemInfo.nCount -= count;
			return true;
		}
	}

	return false;
}

bool CItem::LoadItem(stItem *item)
{
	CSVData::stItem *iteminfo = CSVData::CItemDB::FindById(item->nBaseID);
	if (iteminfo)
	{
		m_pItemDB = iteminfo;
		m_ItemInfo.nBaseID = item->nBaseID;
		m_ItemInfo.nCount = item->nCount;
		m_ItemInfo.nGuid = item->nGuid;
		return true;
	}
	return false;
}
