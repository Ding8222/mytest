#pragma once
#include <string>
#include <assert.h>
#include "State.h"


template <class entity_type>
class CStateMachine
{
public:

	CStateMachine() :m_Owner(nullptr),
		m_CurrentState(nullptr),
		m_PreviousState(nullptr),
		m_GlobalState(nullptr)
	{}

	virtual ~CStateMachine() {}

	void SetOwner(entity_type *owner) { m_Owner = owner; }

	void SetCurrentState(CState<entity_type> *s) { m_CurrentState = s; }
	void SetGlobalState(CState<entity_type> *s) { m_GlobalState = s; }
	void SetPreviousState(CState<entity_type> *s) { m_PreviousState = s; }

	void Update()const
	{
		if (m_GlobalState) m_GlobalState->Execute(m_Owner);

		if (m_CurrentState) m_CurrentState->Execute(m_Owner);
	}

	// 改变状态
	void ChangeState(CState<entity_type> *pNewState)
	{
		assert(pNewState &&
			"<CStateMachine::ChangeState>: 尝试转变为一个nullptr状态");

		m_PreviousState = m_CurrentState;

		m_CurrentState->Exit(m_Owner);

		m_CurrentState = pNewState;

		m_CurrentState->Enter(m_Owner);
	}

	// 返回上一个状态
	void RevertToPreviousState()
	{
		ChangeState(m_PreviousState);
	}

	// 是否处于该状态
	bool isInState(const CState<entity_type>& st)const
	{
		return typeid(*m_CurrentState) == typeid(st);
	}

	CState<entity_type> *CurrentState()  const { return m_CurrentState; }
	CState<entity_type> *GlobalState()   const { return m_GlobalState; }
	CState<entity_type> *PreviousState() const { return m_PreviousState; }

private:
	// 拥有该状态机的实体对象
	entity_type * m_Owner;
	// 当前状态
	CState<entity_type> *m_CurrentState;
	// 上一个状态
	CState<entity_type> *m_PreviousState;
	// 全局状态,每次更新时都调用
	CState<entity_type> *m_GlobalState;
};