#pragma once
#include "Map.h"

class MapDrawFore : public Object2D
{
public:
	MapDrawFore();
	~MapDrawFore() {};

	void Draw() override;
};