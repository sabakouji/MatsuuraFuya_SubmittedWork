#pragma once
#include "Map.h"

class MapDrawBack : public Object2D
{
public:
	MapDrawBack();
	~MapDrawBack() {};

	void Draw() override;
};