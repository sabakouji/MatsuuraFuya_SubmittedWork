#pragma once
#include "EventBase.h"
#include "Animator.h"

class EventRescue : public EventBase
{
public:
	EventRescue();
	virtual ~EventRescue();

	void MakeRescue(TextReader* txt, int n);
	void UpdateEvent() override;

};