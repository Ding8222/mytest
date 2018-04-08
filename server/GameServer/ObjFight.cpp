#include "ObjFight.h"
#include "Timer.h"
#include "BaseObj.h"
#include "CSVLoad.h"

CObjFight::CObjFight()
{
	m_AttackSkill.clear();
	m_HurtMap.clear();
}

CObjFight::~CObjFight()
{
	m_AttackSkill.clear();
	CleanHurtMap();
}

void CObjFight::FightRun()
{
	int64 nNowTime = CTimer::GetTime64();
	UpdateHurtMap(nNowTime);
	ApplySkill(nNowTime);
}

void CObjFight::ActiveAttack(uint32 targettempid, int32 skillid, float x, float y, float z)
{
	int32 nLev = 1;
	int64 key = MAKE_SKILL_KEY(skillid, nLev);
	CSVData::stSkill *skill = CSVData::CSkillDB::FindById(key);
	if (skill)
	{
		m_AttackSkill.clear();
		m_AttackSkill.nSkillID = skillid;
		m_AttackSkill.nLev = nLev;
		m_AttackSkill.nTime = CTimer::GetTime64();
		m_AttackSkill.nTargetTempID = targettempid;
		m_AttackSkill.x = x;
		m_AttackSkill.y = y;
		m_AttackSkill.z = z;
	}
}

void CObjFight::ApplySkill(const int64 &time)
{
	// 检查是否有技能需要使用
	if (m_AttackSkill.isValid())
	{
		// 到达使用时间
		if (time >= m_AttackSkill.nTime)
		{
			int64 key = MAKE_SKILL_KEY(m_AttackSkill.nSkillID, m_AttackSkill.nLev);
			m_AttackSkill.pSkill = CSVData::CSkillDB::FindById(key);
			if (m_AttackSkill.pSkill)
			{
				std::list<CBaseObj *> TargetList;
				// 筛选目标
				FilterTarget(TargetList);
				for (auto &i : TargetList)
				{
					// 计算伤害
					int32 nDamage = CalculateDamage(i);
					if (nDamage > 0)
					{
						// 应用伤害
						ApplyDamage(i, nDamage);
					}
				}
			}
			// 清理技能
			m_AttackSkill.clear();
		}
	}
}

void CObjFight::FilterTarget(std::list<CBaseObj *> &list)
{
	// 有目标
	if (m_AttackSkill.nTargetTempID > 0)
	{
		CBaseObj *target = GetObj()->FindFromAoiList(m_AttackSkill.nTargetTempID);
		list.push_back(target);
	}
}

int32 CObjFight::CalculateDamage(CBaseObj *target)
{
	int32 nDamage = 0;
	CSVData::stSkill *skill = static_cast<CSVData::stSkill *>(m_AttackSkill.pSkill);
	nDamage = skill->nBaseDamage;

	return nDamage;
}

void CObjFight::ApplyDamage(CBaseObj *target, int32 value)
{
	target->ChangeNowHP(value);
	target->AddToHurtMap(GetObj()->GetTempID(), value);
	if (target->GetNowHP() == 0)
	{
		//死亡
		target->Die();
	}
}

// 更新伤害列表
void CObjFight::UpdateHurtMap(const int64 &time)
{
	// 更新伤害列表
	std::map<int32, stHurt *>::iterator iter = m_HurtMap.begin();
	for (; iter != m_HurtMap.end();)
	{
		stHurt *hurt = iter->second;
		if (hurt->isNeedRemove(time))
		{
			delete hurt;
			iter = m_HurtMap.erase(iter);
			continue;
		}
		++iter;
	}
}

void CObjFight::AddToHurtMap(uint32 tempid, int32 value)
{
	std::map<int32, stHurt *>::iterator iter = m_HurtMap.find(tempid);
	if (iter != m_HurtMap.end())
	{
		stHurt *hurt = iter->second;
		hurt->nLastHurtTime = CTimer::GetTime();
		hurt->nHurt += value;
	}
	else
	{
		stHurt *hurt = new stHurt;
		hurt->nLastHurtTime = CTimer::GetTime();
		hurt->nHurt = value;

		m_HurtMap.insert(std::make_pair(tempid, hurt));
	}
}

void CObjFight::CleanHurtMap()
{
	for (auto &i : m_HurtMap)
	{
		delete i.second;
	}
	m_HurtMap.clear();
}
