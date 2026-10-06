#pragma once
#include "Animator.h"
#include "TextReader.h"

#include "EventBase.h"
#include <string>
#include <list>

#include "EventChangeScript.h"
#include "EventMoveMap.h"
#include "EventRescue.h"


class EventManager : public EventBase
{
public:
	EventManager();
	~EventManager();

	void MakeEvent(TextReader* txt, int n);

private:

};