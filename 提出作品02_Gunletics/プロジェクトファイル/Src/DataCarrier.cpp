#include "DataCarrier.h"


DataCarrier::DataCarrier()
{
	ObjectManager::DontDestroy(this);		// DataCarrier‚ÍÁ‚³‚ê‚È‚¢
	ObjectManager::SetVisible(this, false);		// DataCarrier‚Í•\¦‚µ‚È‚¢

	mapNo = 0;
	score = 0;
	second = 0;
	tensecond = 0;
	minute = 0;
}

DataCarrier::~DataCarrier()
{
}

void DataCarrier::Start()
{

}

void DataCarrier::Update()
{

}