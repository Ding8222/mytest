#include "Item.h"
#include "Package.h"
#include "ServerLog.h"
#include "PlayerSaveAndLoadDef.h"

CPackage::CPackage()
{
	m_MaxCount = 0;
	m_Package.clear();
}

CPackage::~CPackage()
{
	m_Package.clear();
}

bool CPackage::Init()
{
	return true;
}

void CPackage::Run()
{

}

void CPackage::Destroy()
{
	for (auto iter : m_Package)
	{
		CItemCreator::ReleaseItem(iter.second);
	}

	m_Package.clear();
}

bool CPackage::Save(char *dest, int32 &retlen)
{
	SAVE_DATA_CHECK();

	int32 len = 0;
	SET_DATA_NUM(PACKAGE_DATA_NUM);
	SET_DATA_VER(PACKAGE_DATA_VER);
	SET_DATA_COUNT(m_Package.size());

	memcpy((void *)(dest + len), &m_MaxCount, sizeof(int32));
	len += sizeof(int32);

	for (auto &iter : m_Package)
	{
		CItem *temp = iter.second;

		memcpy((void *)(dest + len), temp->GetItemInfo(), sizeof(stItem));
		len += sizeof(stItem);
	}

	SET_DATA_LEN;

	return true;
}

bool CPackage::Load(const char *dest, int32 &retlen)
{
	int32 maxsize;
	GET_DATA_CHECK;
	int32 count = GET_DATA_COUNT;
	int32 datanum = GET_DATA_NUM;
	if (GET_DATA_NUM == PACKAGE_DATA_NUM)
	{
		int32 offset = s_Data_Base_Size;
		int8 nVer = GET_DATA_VER;
		int32 nDataSize;
		switch (nVer)
		{
		case PACKAGE_DATA_VER:
		{
			nDataSize = sizeof(stItem);
			break;
		}
		default: return false;
		}

		if (maxsize < int32(count * nDataSize + offset))return false;

		switch (nVer)
		{
			case PACKAGE_DATA_VER:
			{
				memcpy(&m_MaxCount, (void *)(dest + offset), sizeof(int32));
				offset += sizeof(int32);

				for (int32 i = 0; i < count; i++)
				{
					stItem *_data = (stItem *)(dest + offset);
					if (_data)
					{
						CItem *item = CItemCreator::CreateItem();
						if (item)
						{
							if (!item->LoadItem(_data))
							{
								return false;
							}
							m_Package[item->GetGuid()] = item;
						}
					}
					offset += nDataSize;
				}
				break;
			default: return false;
			}
		}
		retlen = offset;
	}
	return true;
}

bool CPackage::AddItem(int32 itemid, int32 count)
{
	CItem *item = CItemCreator::CreateItem();
	if (item)
	{
		if (item->Init(itemid, count))
		{
			if (AddItem(item))
				return true;
			else
			{
				CItemCreator::ReleaseItem(item);
			}
		}
		else
		{
			CItemCreator::ReleaseItem(item);
		}
	}

	return false;
}

bool CPackage::AddItem(CItem *item)
{
	if (!item)
		return false;

	auto iter = m_Package.find(item->GetGuid());
	if (iter == m_Package.end())
	{
		m_Package.insert(std::make_pair(item->GetGuid(), item));
		return true;
	}
	else
	{
		RunStateError("添加道具到背包失败！重复的GUID：%I64d", item->GetGuid());
	}

	return false;
}

bool CPackage::DelItem(int32 itemid, int32 count)
{
	int32 leftcount = count;
	CItem *item;
	std::unordered_map<int64, CItem*>::iterator iter, tempiter;
	for (iter = m_Package.begin(); iter != m_Package.end();)
	{
		if (leftcount <= 0)
			break;

		item = iter->second;
		++iter;

		if (item->GetBaseID() == itemid)
		{
			if (item->GetCount() > leftcount)
			{
				// 单个道具数量大于扣除数量
				if(item->CountChange(leftcount))
					leftcount = 0;
				break;
			}
			else if (item->GetCount() == leftcount)
			{
				// 单个道具数量等于扣除数量
				CItemCreator::ReleaseItem(item);
				m_Package.erase(iter);
				leftcount = 0;
				break;
			}
			else
			{
				// 单个道具数量小于扣除数量
				leftcount -= item->GetCount();
				CItemCreator::ReleaseItem(item);
				m_Package.erase(iter);
			}
		}
	}

	return leftcount == 0;
}

bool CPackage::DelItem(int64 guid, int32 count)
{
	auto iter = m_Package.find(guid);
	if (iter != m_Package.end())
	{
		CItemCreator::ReleaseItem(iter->second);
		m_Package.erase(iter);
		return true;
	}

	return false;
}

CItem *CPackage::GetItem(int64 guid)
{
	auto iter = m_Package.find(guid);
	if (iter != m_Package.end())
	{
		return iter->second;
	}

	return nullptr;
}
