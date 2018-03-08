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

	// 自定义结构体
	struct stExample
	{
		stExample()
		{
			nIndex = 0;
			nGold = 0;
			text.clear();
		}
		int nIndex;
		double nGold;
		std::string text;
	};
	
	class CExample
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
}