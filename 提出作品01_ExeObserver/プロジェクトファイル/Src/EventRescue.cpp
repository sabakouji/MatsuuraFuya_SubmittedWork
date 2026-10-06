#include "EventManager.h"
#include "Player.h"
#include "AudioManager.h"


EventRescue::EventRescue()
{
}

EventRescue::~EventRescue()
{
}

void EventRescue::MakeRescue(TextReader* txt, int n)
{
	tag = txt->GetString(n, 1);
	if (txt->GetColumns(n) > 2)
	{
		if ( txt->GetString(n, 2) == "Once") onceOn = true;
	}
	if (txt->GetColumns(n) > 3)
	{
		if (txt->GetString(n, 3) == "Enter") needEnter = true;
	}
}

void EventRescue::UpdateEvent()
{
	ObjectManager::FindGameObject<Player>()->SetMaxHp();
}

