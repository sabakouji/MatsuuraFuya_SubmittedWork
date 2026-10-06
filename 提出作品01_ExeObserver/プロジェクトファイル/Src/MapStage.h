#pragma once
#include "MapManager.h"

class MapStage : public MapBase {
public:
	MapStage();
	~MapStage();

	void MakeStageMap(TextReader* txt, int n);

private:
};