/*
* CSV加载
* Copyright (C) ddl
* 2018
*/
#pragma once

#include <string>
#include <unordered_map>
#include "CSVParser.h"

#define DEFINE_CLEAR static void Destroy(){ for( auto i : m_Data ) delete(i.second); m_Data.clear();}
#define MAKE_SKILL_KEY(id,lev) (((int64)(id)<<8) | (lev))
#define MAKE_STATUS_KEY(id,lev) (((int64)(id)<<8) | (lev))

namespace CSVData
{
	bool Init();
	void Destroy();

	// 例子

	// 自定义结构体
	struct stExample
	{
		stExample()
		{
			nIndex = 0;
			nGold = 0;
			text.clear();
		}
		int32 nIndex;
		double nGold;
		std::string text;
	};
	
	class CExampleDB
	{
	public:
		// 会被CSVParser调用,不要修改
		static bool AddData(CSV::Row & _Row);
		// m_Data中的查找,可根据实际情况修改
		static stExample *FindById(int64 _Key)
		{
			std::unordered_map<int64, stExample *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		// 定义m_Data的析构,每个结构体中都必须有这个,并在Destroy()中主动调用
		// 复杂的m_Data清理请自行重写
		DEFINE_CLEAR

		static std::unordered_map<int64, stExample *> m_Data;
	};

	// 技能
	struct stSkill
	{
		stSkill()
		{
			nIndex = 0;
			nSkillID = 0;
			nSkillLev = 0;
			nBaseDamage = 0;
		}
		int32 nIndex;
		int32 nSkillID;
		int32 nSkillLev;
		int32 nBaseDamage;
	};

	class CSkillDB
	{
	public:
		static bool AddData(CSV::Row & _Row);
		static stSkill *FindById(int64 _Key)
		{
			std::unordered_map<int64, stSkill *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		DEFINE_CLEAR

		static std::unordered_map<int64, stSkill *> m_Data;
	};

	// 状态
	struct stStatus
	{
		stStatus()
		{
			nIndex = 0;
			nStatusID = 0;
			nStatusLev = 0;
		}
		int32 nIndex;
		int32 nStatusID;
		int32 nStatusLev;
	};

	class CStatusDB
	{
	public:
		static bool AddData(CSV::Row & _Row);
		static stStatus *FindById(int64 _Key)
		{
			std::unordered_map<int64, stStatus *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		DEFINE_CLEAR

		static std::unordered_map<int64, stStatus *> m_Data;
	};

	// 地图
	struct stMap
	{
		stMap()
		{
			nMapID = 0;
			nLineID = 0;
			nType = 0;
			nX = 0.0f;
			nY = 0.0f;
			nZ = 0.0f;
			sMapBar.clear();
		}
		int32 nMapID;
		int32 nLineID;
		int32 nType;
		float nX;
		float nY;
		float nZ;
		std::string sMapBar;
	};

	class CMapDB
	{
	public:
		static bool AddData(CSV::Row & _Row);
		static stMap *FindById(int64 _Key)
		{
			std::unordered_map<int64, stMap *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		DEFINE_CLEAR

		static std::unordered_map<int64, stMap *> m_Data;
	};

	// 传送门
	struct stMapGate
	{
		stMapGate()
		{
			nMapGateID = 0;
			nSrcMapID = 0;
			nSrcX = 0.0f;
			nSrcY = 0.0f;
			nSrcZ = 0.0f;
			nTarMapID = 0;
			nTarX = 0.0f;
			nTarY = 0.0f;
			nTarZ = 0.0f;
		}
		int32 nMapGateID;
		// 所在地图
		int32 nSrcMapID;
		float nSrcX;
		float nSrcY;
		float nSrcZ;
		// 目标地图
		int32 nTarMapID;
		float nTarX;
		float nTarY;
		float nTarZ;
	};

	class CMapGateDB
	{
	public:
		static bool AddData(CSV::Row & _Row);
		static stMapGate *FindById(int64 _Key)
		{
			std::unordered_map<int64, stMapGate *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		DEFINE_CLEAR

		static std::unordered_map<int64, stMapGate *> m_Data;
	};

	// 副本
	struct stInstance
	{
		stInstance()
		{
			nInstanceID = 0;
			nMapID = 0;
			nLimitTime = 0;
		}
		int32 nInstanceID;
		int32 nMapID;
		int32 nLimitTime;
	};

	class CInstanceDB
	{
	public:
		static bool AddData(CSV::Row & _Row);
		static stInstance *FindById(int64 _Key)
		{
			std::unordered_map<int64, stInstance *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		DEFINE_CLEAR

		static std::unordered_map<int64, stInstance *> m_Data;
	};

	// Monster
	struct stMonster
	{
		stMonster()
		{
			nMonsterID = 0;
			nMonsterType = 0;
			Name.clear();
		}
		int32 nMonsterID;
		int32 nMonsterType;
		std::string Name;
	};

	class CMonsterDB
	{
	public:
		static bool AddData(CSV::Row & _Row);
		static stMonster *FindById(int64 _Key)
		{
			std::unordered_map<int64, stMonster *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		DEFINE_CLEAR

		static std::unordered_map<int64, stMonster *> m_Data;
	};

	// NPC
	struct stNPC
	{
		stNPC()
		{
			nNPCID = 0;
			nNPCType = 0;
			Name.clear();
			nMapID = 0;
			nX = 0.0f;
			nY = 0.0f;
			nZ = 0.0f;
		}
		int32 nNPCID;
		int32 nNPCType;
		std::string Name;
		int32 nMapID;
		float nX;
		float nY;
		float nZ;
	};

	class CNPCDB
	{
	public:
		static bool AddData(CSV::Row & _Row);
		static stNPC *FindById(int64 _Key)
		{
			std::unordered_map<int64, stNPC *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		DEFINE_CLEAR

		static std::unordered_map<int64, stNPC *> m_Data;
	};

	// 地图刷怪
	struct stMapMonster
	{
		stMapMonster()
		{
			nMapID = 0;
			nMonsterID = 0;
			bCanRelive = false;
			nReliveCD = 0;
			nX = 0.0f;
			nY = 0.0f;
			nZ = 0.0f;
		}
		int32 nMapID;
		int32 nMonsterID;
		bool bCanRelive;
		int32 nReliveCD;
		float nX;
		float nY;
		float nZ;
	};

	class CMapMonsterDB
	{
	public:
		static bool AddData(CSV::Row & _Row);
		static std::vector<stMapMonster *> *FindById(int64 _Key)
		{
			std::unordered_map<int64, std::vector<stMapMonster *> *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		static void Destroy()
		{
			for (auto &i : m_Data)
			{
				std::vector<stMapMonster *> *monster = i.second;
				std::vector<stMapMonster *>::iterator iter = monster->begin();
				for (; iter != monster->end(); ++iter)
				{
					delete *iter;
				}
				monster->clear();
				delete monster;
			}
			m_Data.clear();
		}

		static std::unordered_map<int64, std::vector<stMapMonster *> *> m_Data;
	};

	// 副本刷怪
	struct stInstanceMonster
	{
		stInstanceMonster()
		{
			nInstanceID = 0;
			nMonsterID = 0;
			bCanRelive = false;
			nReliveCD = 0;
			nX = 0.0f;
			nY = 0.0f;
			nZ = 0.0f;
		}
		int32 nInstanceID;
		int32 nMonsterID;
		bool bCanRelive;
		int32 nReliveCD;
		float nX;
		float nY;
		float nZ;
	};

	class CInstanceMonsterDB
	{
	public:
		static bool AddData(CSV::Row & _Row);
		static std::vector<stInstanceMonster *> *FindById(int64 _Key)
		{
			std::unordered_map<int64, std::vector<stInstanceMonster *> *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		DEFINE_CLEAR

		static std::unordered_map<int64, std::vector<stInstanceMonster *> *> m_Data;
	};
}