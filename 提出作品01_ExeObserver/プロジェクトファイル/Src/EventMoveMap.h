#pragma once
#include "EventBase.h"
#include "Animator.h"

class EventMoveMap : public EventBase
{
public:
	EventMoveMap();
	virtual ~EventMoveMap();

	void MakeMoveMap(TextReader* txt, int n);
	void UpdateEvent() override;

private:
	std::string tagControl;
	std::string activeMove;

};