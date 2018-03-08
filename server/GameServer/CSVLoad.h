/*
* CSV加载
* Copyright (C) ddl
* 2018
*/
#pragma once

#include "CSVParser.h"
#include <string>
#include <unordered_map>

#define  DEFINE_CLEAR static void Destroy(){ for( auto i : m_Data ) delete(i.second); m_Data.clear();}

namespace CSVData
{
	bool Init();
	void Destroy();

	// 例子
	struct Example
	{
		Example()
		{
			nIndex = 0;
			nGold = 0;
			text.clear();
		}
		int nIndex;
		double nGold;
		std::string text;

		static bool AddData(CSV::Row & _Row);
		static Example* FindById(int64 _Key)
		{
			std::unordered_map<int64, Example *>::iterator iter = m_Data.find(_Key);
			if (iter != m_Data.end())
				return iter->second;

			return nullptr;
		}

		DEFINE_CLEAR
		static std::unordered_map<int64, Example *> m_Data;
	};
}