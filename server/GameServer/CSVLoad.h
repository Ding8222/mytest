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
#define MAKE_SKILL_KEY(id,lev) (((id)<<8) | (lev))
#define MAKE_STATUS_KEY(id,lev) (((id)<<8) | (lev))

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
		static stExample* FindById(int64 _Key)
		{
			std::unordered_map<int64, stExample *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		//定义m_Data的析构,每个结构体中都必须有这个,并在Destroy()中主动调用
		DEFINE_CLEAR
	private:
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
		static stSkill* FindById(int64 _Key)
		{
			std::unordered_map<int64, stSkill *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		DEFINE_CLEAR
	private:
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
		static stStatus* FindById(int64 _Key)
		{
			std::unordered_map<int64, stStatus *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}
		DEFINE_CLEAR
	private:
		static std::unordered_map<int64, stStatus *> m_Data;
	};
}