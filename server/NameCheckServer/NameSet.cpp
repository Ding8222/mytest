#include "NameSet.h"

CNameSet::CNameSet()
{
	m_NameSet.clear();
}

CNameSet::~CNameSet()
{
	Destroy();
}

void CNameSet::Destroy()
{
	m_NameSet.clear();
}

bool CNameSet::AddName(const std::string &name)
{
	auto iter = m_NameSet.find(name);
	if (iter == m_NameSet.end())
	{
		m_NameSet.insert(name);
		return true;
	}
	return false;
}
