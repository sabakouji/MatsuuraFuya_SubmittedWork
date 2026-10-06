#pragma once
#include "Object2D.h"
#include "DInput.h"

/// <summary>
/// ‰æ–Ê‚ÉŠeíî•ñ‚ğ•\¦‚·‚éˆ—
/// </summary>
class Plater;
class DisplayInfo : public Object2D
{
public:
	DisplayInfo(CSpriteImage* inimage);
	~DisplayInfo();

	void Update() override;
	void Draw() override;
private:
	CSpriteImage* image;

	int HPX;
	int HPY;
	int HPwi;
	int HPhi;
	int PLHP;
	int hpsegment;
	int currentheigth3, currentheigth2, currentheigth1;
	int currentSouce3,currentSouce2, currentSouce1;
	int currentDest3, currentDest2, currentDest1;
};