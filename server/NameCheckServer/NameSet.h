/*
* 名称集合
* Copyright (C) ddl
* 2018
*/
#pragma once

#include <unordered_set>
class CNameSet
{
public:
	CNameSet();
	~CNameSet();

	static CNameSet &Instance()
	{
		static CNameSet m;
		return m;
	}

	void Destroy();

	bool AddName(const std::string &name);

private:

	std::unordered_set<std::string> m_NameSet;
};