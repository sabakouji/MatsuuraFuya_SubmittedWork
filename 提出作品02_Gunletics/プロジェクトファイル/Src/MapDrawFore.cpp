#include "MapDrawFore.h"

MapDrawFore::MapDrawFore()
{
	SetDrawOrder(-90);	   // àÍî‘ç≈å„Ç…ï`âÊÇ∑ÇÈ
	ObjectManager::SetActive(this, false);
}

void MapDrawFore::Draw()
{
	Map* obj = dynamic_cast<Map*>(Parent());
	obj->DrawFore();
	if (obj->DrawMaplineOK()) obj->DrawMapLine();
	if (obj->DrawCoordOK()) obj->DrawCoord();
}