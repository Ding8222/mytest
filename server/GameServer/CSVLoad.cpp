#include "CSVLoad.h"
#include "serverlog.h"
#include "fmt/ostream.h"

namespace CSVData
{
	static std::string text;
	// 读取CSV
	bool Init()
	{
		if (!CSV::CsvLoader::LoadCsv<CExampleDB>("Example"))
			return false;
		
		if (!CSV::CsvLoader::LoadCsv<CSkillDB>("Skill"))
			return false;

		if (!CSV::CsvLoader::LoadCsv<CStatusDB>("Status"))
			return false;

		if (!CSV::CsvLoader::LoadCsv<CMapDB>("Map"))
			return false;

		if (!CSV::CsvLoader::LoadCsv<CMapGateDB>("MapGate"))
			return false;

		if (!CSV::CsvLoader::LoadCsv<CInstanceDB>("Instance"))
			return false;

		if (!CSV::CsvLoader::LoadCsv<CMonsterDB>("Monster"))
			return false;

		if (!CSV::CsvLoader::LoadCsv<CNPCDB>("NPC"))
			return false;

		if (!CSV::CsvLoader::LoadCsv<CMapMonsterDB>("MapMonster"))
			return false;

		if (!CSV::CsvLoader::LoadCsv<CInstanceMonsterDB>("InstanceMonster"))
			return false;

		if (!CSV::CsvLoader::LoadCsv<CItemDB>("Item"))
			return false;

		RunStateLog("加载所有CSV成功！");
		return true;
	}

	// 需要释放申请的资源
	void Destroy()
	{
		CExampleDB::Destroy();
		CSkillDB::Destroy();
		CStatusDB::Destroy();
		CMapDB::Destroy();
		CMapGateDB::Destroy();
		CInstanceDB::Destroy();
		CMonsterDB::Destroy();
		CNPCDB::Destroy();
		CMapMonsterDB::Destroy();
		CInstanceMonsterDB::Destroy();
		CItemDB::Destroy();
	}

	// 例子
	std::unordered_map<int64, stExample *> CExampleDB::m_Data;
	bool CExampleDB::AddData(CSV::Row & _Row)
	{
		stExample *pdata = new stExample;
		_Row.getValue(pdata->nIndex, "序号");
		_Row.getValue(pdata->nGold, "金额");
		_Row.getValue(pdata->text, "描述");

		if (FindById(pdata->nIndex))
		{
			RunStateError("添加重复项目 %d ！", pdata->nIndex);
			delete pdata;
			return false;
		}

		m_Data.insert(std::make_pair(pdata->nIndex, pdata));
		return true;
	}

	// 技能
	std::unordered_map<int64, stSkill *> CSkillDB::m_Data;
	bool CSkillDB::AddData(CSV::Row & _Row)
	{
		stSkill *pdata = new stSkill;
		_Row.getValue(pdata->nIndex, "Index");
		_Row.getValue(pdata->nSkillID, "技能ID");
		_Row.getValue(pdata->nSkillLev, "技能等级");
		_Row.getValue(pdata->nBaseDamage, "基础伤害");

		int64 key = MAKE_SKILL_KEY(pdata->nSkillID, pdata->nSkillLev);
		if (FindById(key))
		{
			RunStateError("添加重复项目 %d ！", pdata->nIndex);
			delete pdata;
			return false;
		}

		m_Data.insert(std::make_pair(key, pdata));
		return true;
	}

	// 状态
	std::unordered_map<int64, stStatus *> CStatusDB::m_Data;
	bool CStatusDB::AddData(CSV::Row & _Row)
	{
		stStatus *pdata = new stStatus;
		_Row.getValue(pdata->nIndex, "Index");
		_Row.getValue(pdata->nStatusID, "状态ID");
		_Row.getValue(pdata->nStatusLev, "状态等级");
		_Row.getValue(pdata->nTime, "持续时间");

		int64 key = MAKE_STATUS_KEY(pdata->nStatusID, pdata->nStatusLev);
		if (FindById(key))
		{
			RunStateError("添加重复项目 %d ！", pdata->nIndex);
			delete pdata;
			return false;
		}

		m_Data.insert(std::make_pair(key, pdata));
		return true;
	}

	// 副本
	std::unordered_map<int64, stInstance *> CInstanceDB::m_Data;
	bool CInstanceDB::AddData(CSV::Row & _Row)
	{
		stInstance *pdata = new stInstance;
		_Row.getValue(pdata->nInstanceID, "副本ID");
		_Row.getValue(pdata->nMapID, "地图ID");
		_Row.getValue(pdata->nLimitTime, "时间限制");
		_Row.getValue(pdata->nMaxWave, "最大波数");

		if (FindById(pdata->nInstanceID))
		{
			RunStateError("添加重复项目 %d ！", pdata->nInstanceID);
			delete pdata;
			return false;
		}

		if (pdata->nInstanceID < 0 || pdata->nMapID < 0 || pdata->nLimitTime <= 0 || pdata->nMaxWave <= 0)
		{
			RunStateError("配置错误 %d ！", pdata->nInstanceID);
			delete pdata;
			return false;
		}

		m_Data.insert(std::make_pair(pdata->nInstanceID, pdata));
		return true;
	}

	// 地图
	std::unordered_map<int64, stMap *> CMapDB::m_Data;
	bool CMapDB::AddData(CSV::Row & _Row)
	{
		stMap *pdata = new stMap;
		_Row.getValue(pdata->nMapID, "地图ID");
		_Row.getValue(pdata->nLineID, "线路ID");
		_Row.getValue(pdata->nType, "地图类型");
		_Row.getValue(pdata->nX, "出生点X");
		_Row.getValue(pdata->nY, "出生点Y");
		_Row.getValue(pdata->nZ, "出生点Z");
		_Row.getValue(pdata->sMapBar, "阻挡文件路径");

		if (FindById(pdata->nMapID))
		{
			RunStateError("添加重复项目 %d ！", pdata->nMapID);
			delete pdata;
			return false;
		}

		if (pdata->nMapID < 0 || pdata->nLineID < 0 || pdata->sMapBar.empty())
		{
			RunStateError("配置错误 %d ！", pdata->nMapID);
			delete pdata;
			return false;
		}

		m_Data.insert(std::make_pair(pdata->nMapID, pdata));
		return true;
	}

	// 地图
	std::unordered_map<int64, stMapGate *> CMapGateDB::m_Data;
	bool CMapGateDB::AddData(CSV::Row & _Row)
	{
		stMapGate *pdata = new stMapGate;
		_Row.getValue(pdata->nMapGateID, "传送门ID");
		_Row.getValue(pdata->nSrcMapID, "所在地图");
		_Row.getValue(pdata->nSrcX, "所在地图坐标X");
		_Row.getValue(pdata->nSrcY, "所在地图坐标Y");
		_Row.getValue(pdata->nSrcZ, "所在地图坐标Z");
		_Row.getValue(pdata->nTarMapID, "目标地图");
		_Row.getValue(pdata->nTarX, "目标地图坐标X");
		_Row.getValue(pdata->nTarY, "目标地图坐标Y");
		_Row.getValue(pdata->nTarZ, "目标地图坐标Z");

		if (FindById(pdata->nMapGateID))
		{
			RunStateError("添加重复项目 %d ！", pdata->nMapGateID);
			delete pdata;
			return false;
		}
		
		if (pdata->nMapGateID < 0 || pdata->nSrcMapID < 0 || pdata->nTarMapID < 0)
		{
			RunStateError("配置错误 %d ！", pdata->nMapGateID);
			delete pdata;
			return false;
		}

		m_Data.insert(std::make_pair(pdata->nMapGateID, pdata));

		return true;
	}

	// Monster
	std::unordered_map<int64, stMonster *> CMonsterDB::m_Data;
	bool CMonsterDB::AddData(CSV::Row & _Row)
	{
		stMonster *pdata = new stMonster;
		_Row.getValue(pdata->nMonsterID, "怪物ID");
		_Row.getValue(pdata->nMonsterType, "怪物类型");
		_Row.getValue(pdata->Name, "怪物名称");

		if (FindById(pdata->nMonsterID))
		{
			RunStateError("添加重复项目 %d ！", pdata->nMonsterID);
			delete pdata;
			return false;
		}

		if (pdata->nMonsterID < 0 || pdata->nMonsterType < 0)
		{
			RunStateError("配置错误 %d ！", pdata->nMonsterID);
			delete pdata;
			return false;
		}

		m_Data.insert(std::make_pair(pdata->nMonsterID, pdata));
		return true;
	}

	// NPC
	std::unordered_map<int64, stNPC *> CNPCDB::m_Data;
	bool CNPCDB::AddData(CSV::Row & _Row)
	{
		stNPC *pdata = new stNPC;
		_Row.getValue(pdata->nNPCID, "NPCID");
		_Row.getValue(pdata->nNPCType, "NPC类型");
		_Row.getValue(pdata->Name, "NPC名称");
		_Row.getValue(pdata->nMapID, "地图ID");
		_Row.getValue(pdata->nX, "坐标X");
		_Row.getValue(pdata->nY, "坐标Y");
		_Row.getValue(pdata->nZ, "坐标Z");

		if (FindById(pdata->nNPCID))
		{
			RunStateError("添加重复项目 %d ！", pdata->nNPCID);
			delete pdata;
			return false;
		}

		if (pdata->nNPCID < 0 || pdata->nNPCType < 0 || pdata->nMapID < 0)
		{
			RunStateError("配置错误 %d ！", pdata->nNPCID);
			delete pdata;
			return false;
		}

		m_Data.insert(std::make_pair(pdata->nNPCID, pdata));
		return true;
	}

	// 地图刷怪
	std::unordered_map<int64, std::vector<stMapMonster *> *> CMapMonsterDB::m_Data;
	bool CMapMonsterDB::AddData(CSV::Row & _Row)
	{
		stMapMonster *pdata = new stMapMonster;
		_Row.getValue(pdata->nMapID, "地图ID");
		_Row.getValue(pdata->nMonsterID, "怪物ID");
		_Row.getValue(pdata->bCanRelive, "是否复活");
		_Row.getValue(pdata->nReliveCD, "复活CD");
		_Row.getValue(pdata->nX, "坐标X");
		_Row.getValue(pdata->nY, "坐标Y");
		_Row.getValue(pdata->nZ, "坐标Z");

		std::vector<stMapMonster *> *monsterset = FindById(pdata->nMapID);
		if (!monsterset)
		{
			monsterset = new std::vector<stMapMonster *>;
			m_Data.insert(std::make_pair(pdata->nMapID, monsterset));
		}

		if (pdata->nMapID < 0 || pdata->nMonsterID < 0 || pdata->nReliveCD < 0)
		{
			RunStateError("配置错误 %d ！", pdata->nMapID);
			delete pdata;
			return false;
		}

		monsterset->push_back(pdata);
		return true;
	}

	// 副本刷怪
	std::unordered_map<int64, std::list<stInstanceMonster *> *> CInstanceMonsterDB::m_Data;
	bool CInstanceMonsterDB::AddData(CSV::Row & _Row)
	{
		stInstanceMonster *pdata = new stInstanceMonster;
		_Row.getValue(pdata->nInstanceID, "副本ID");
		_Row.getValue(pdata->nWave, "波数");
		_Row.getValue(pdata->nMonsterID, "怪物ID");
		_Row.getValue(pdata->nX, "坐标X");
		_Row.getValue(pdata->nY, "坐标Y");
		_Row.getValue(pdata->nZ, "坐标Z");

		std::list<stInstanceMonster *> *monsterset = FindById(pdata->nInstanceID);
		if (!monsterset)
		{
			monsterset = new std::list<stInstanceMonster *>;
			m_Data.insert(std::make_pair(pdata->nInstanceID, monsterset));
		}

		if (pdata->nInstanceID < 0 || pdata->nMonsterID < 0 || pdata->nWave <= 0)
		{
			RunStateError("配置错误 %d ！", pdata->nInstanceID);
			delete pdata;
			return false;
		}

		monsterset->push_back(pdata);
		return true;
	}

	// Item
	std::unordered_map<int64, stItem *> CItemDB::m_Data;
	bool CItemDB::AddData(CSV::Row & _Row)
	{
		stItem *pdata = new stItem;
		_Row.getValue(pdata->nItemID, "道具ID");
		_Row.getValue(pdata->Name, "道具名称");
		_Row.getValue(pdata->nItemType, "道具类型");
		_Row.getValue(pdata->nMaxCount, "最大叠加数量");

		if (FindById(pdata->nItemID))
		{
			RunStateError("添加重复项目 %d ！", pdata->nItemID);
			delete pdata;
			return false;
		}

		if (pdata->nItemID <= 0 || pdata->nItemType < 0 || pdata->nMaxCount <= 0)
		{
			RunStateError("配置错误 %d ！", pdata->nItemID);
			delete pdata;
			return false;
		}

		m_Data.insert(std::make_pair(pdata->nItemID, pdata));
		return true;
	}
}
