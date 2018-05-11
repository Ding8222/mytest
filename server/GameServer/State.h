#pragma once

template <class entity_type>
class CState
{
public:

	virtual ~CState() {}

	virtual void Enter(entity_type*) = 0;

	virtual void Execute(entity_type*) = 0;

	virtual void Exit(entity_type*) = 0;
};
