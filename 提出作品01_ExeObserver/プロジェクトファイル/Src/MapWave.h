#pragma once
#include "Displace.h"
#include "MapManager.h"


class MapWave : public MapBase {
public:
	MapWave();
	~MapWave();

	void MakeWaveMap(TextReader* txt, int n);

	void Draw() override;

private:
	CWave* wave;
};