#pragma once
#include "EventBase.h"
#include "Animator.h"

class EventChangeScript : public EventBase
{
public:
	EventChangeScript();
	virtual ~EventChangeScript();

	void MakeChangeScript(TextReader* txt, int n);
	void UpdateEvent() override;

private:
	std::string scriptName;
};