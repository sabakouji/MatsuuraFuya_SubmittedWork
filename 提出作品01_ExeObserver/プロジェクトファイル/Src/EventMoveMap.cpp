#include "EventManager.h"
#include "EventMoveMap.h"
#include "PlayScene.h"
#include "Player.h"
#include "DataCarrier.h"
#include "MapMove.h"
#include "AudioManager.h"


EventMoveMap::EventMoveMap()
{
	tagControl = "";
	activeMove = "";
}

EventMoveMap::~EventMoveMap()
{
	SAFE_DELETE(mesh);
	SAFE_DELETE(meshCol);
}

void EventMoveMap::MakeMoveMap(TextReader* txt, int n)
{
	tag = txt->GetString(n, 1);
	tagControl = txt->GetString(n, 2);
	activeMove = txt->GetString(n, 3);
	if (txt->GetColumns(n) > 4)
	{
		if ( txt->GetString(n, 4) == "Once") onceOn = true;
	}
	if (txt->GetColumns(n) > 5)
	{
		if (txt->GetString(n, 5) == "Enter" ) needEnter = true;
	}
}

void EventMoveMap::UpdateEvent()
{
	MapMove* mm = ObjectManager::FindGameObjectWithTag<MapMove>(tagControl);
	if (mm == nullptr)	return;

	if (activeMove == "Active" || activeMove == "ActiveOn")
	{
		mm->SetActiveOn(true);
	}
	else if (activeMove == "ActiveOff")
	{
		mm->SetActiveOn(false);
	}else if (activeMove == "MoveOn")
	{
		mm->SetActiveOn(true);
		mm->SetMoveOn(true);
	}else if (activeMove == "MoveOff")
	{
		mm->SetActiveOn(true);
		mm->SetMoveOn(false);
	}
}

