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

		if (!CSV::CsvLoader::LoadCsv<CMonsterDB>("Monster"))
			return false;

		if (!CSV::CsvLoader::LoadCsv<CNPCDB>("NPC"))
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
		CMonsterDB::Destroy();
		CNPCDB::Destroy();
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
		_Row.getValue(pdata->nIndex, "序号");
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
		_Row.getValue(pdata->nIndex, "序号");
		_Row.getValue(pdata->nStatusID, "状态ID");
		_Row.getValue(pdata->nStatusLev, "状态等级");

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

	// Monster
	std::unordered_map<int64, stMonster *> CMonsterDB::m_Data;
	bool CMonsterDB::AddData(CSV::Row & _Row)
	{
		stMonster *pdata = new stMonster;
		_Row.getValue(pdata->nMonsterID, "怪物ID");
		_Row.getValue(pdata->nMonsterType, "怪物类型");
		_Row.getValue(pdata->bCanRelive, "是否复活");
		_Row.getValue(pdata->nReliveCD, "复活CD");
		_Row.getValue(pdata->nMapID, "地图ID");
		_Row.getValue(pdata->nX, "坐标X");
		_Row.getValue(pdata->nY, "坐标Y");
		_Row.getValue(pdata->nZ, "坐标Z");

		if (FindById(pdata->nMonsterID))
		{
			RunStateError("添加重复项目 %d ！", pdata->nMonsterID);
			delete pdata;
			return false;
		}

		if (pdata->nMonsterID < 0 || pdata->nMonsterType < 0 || pdata->nMapID < 0 || pdata->nReliveCD < 0)
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
}