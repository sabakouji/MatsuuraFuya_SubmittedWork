#pragma once
#include "MapManager.h"

class MapSky : public MapBase {
public:
	MapSky();
	~MapSky();

	void MakeSkyMap(TextReader* txt, int n);
	void Update() override;
	void Draw() override;

private:

};