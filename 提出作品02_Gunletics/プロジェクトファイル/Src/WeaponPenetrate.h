#pragma once
#include "Object2D.h"
#include "Animator.h"
#include "Collider.h"
#include "WeaponBase.h"
#include "Map.h"

class WeaponPenetrate : public WeaponBase
{
public:
	WeaponPenetrate(CSpriteImage* image);
	~WeaponPenetrate();
	void Start() override;
	void Update() override;
	void Draw() override;

	int mgsize;

private:
	MapLine* pHitMapLine;
	Map* objMap;
	VECTOR2 velocity;

	void updateNormal();
};