#include "MapDrawBack.h"

MapDrawBack::MapDrawBack()
{
	SetDrawOrder(100);	   // ˆê”ÔÅ‰‚É•`‰æ‚·‚é
	ObjectManager::SetActive(this, false);
}

void MapDrawBack::Draw()
{
	Map* obj = dynamic_cast<Map*>(Parent());
	obj->DrawBack();
}