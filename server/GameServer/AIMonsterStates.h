#pragma once
#include "State.h"

class CAIMonster;

// 靠近
class CSeek : public CState<CAIMonster>
{
private:

	CSeek() {}

	CSeek(const CSeek&);
	CSeek& operator=(const CSeek&);

public:

	static CSeek* Instance();

public:

	virtual void Enter(CAIMonster* pMonster);

	virtual void Execute(CAIMonster* pMonster);

	virtual void Exit(CAIMonster* pMonster);

};

// 徘徊
class CWander : public CState<CAIMonster>
{
private:

	CWander() {}

	CWander(const CWander&);
	CWander& operator=(const CWander&);

public:

	static CWander* Instance();

public:

	virtual void Enter(CAIMonster* pMonster);

	virtual void Execute(CAIMonster* pMonster);

	virtual void Exit(CAIMonster* pMonster);

};

// 追逐
class CPursuit : public CState<CAIMonster>
{
private:

	CPursuit() {}

	CPursuit(const CPursuit&);
	CPursuit& operator=(const CPursuit&);

public:

	static CPursuit* Instance();

	virtual void Enter(CAIMonster* pMonster);

	virtual void Execute(CAIMonster* pMonster);

	virtual void Exit(CAIMonster* pMonster);
};

// 逃避
class CEvade : public CState<CAIMonster>
{
private:

	CEvade() {}

	CEvade(const CEvade&);
	CEvade& operator=(const CEvade&);

public:

	static CEvade* Instance();

	virtual void Enter(CAIMonster* pMonster);

	virtual void Execute(CAIMonster* pMonster);

	virtual void Exit(CAIMonster* pMonster);
};

// 返回出生点
class CReturnToHomeRegion : public CState<CAIMonster>
{
private:

	CReturnToHomeRegion() {}

	CReturnToHomeRegion(const CReturnToHomeRegion&);
	CReturnToHomeRegion& operator=(const CReturnToHomeRegion&);

public:

	static CReturnToHomeRegion* Instance();

	virtual void Enter(CAIMonster* pMonster);

	virtual void Execute(CAIMonster* pMonster);

	virtual void Exit(CAIMonster* pMonster);
};

// 攻击
class CAttack : public CState<CAIMonster>
{
private:

	CAttack() {}

	CAttack(const CAttack&);
	CAttack& operator=(const CAttack&);

public:

	static CAttack* Instance();

	virtual void Enter(CAIMonster* pMonster);

	virtual void Execute(CAIMonster* pMonster);

	virtual void Exit(CAIMonster* pMonster);
};
