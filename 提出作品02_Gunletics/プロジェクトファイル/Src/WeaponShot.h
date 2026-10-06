#pragma once
#include "Object2D.h"
#include "Animator.h"
#include "Collider.h"
#include "WeaponManager.h"
#include "Map.h"

/// <summary>
/// 武器ショットのクラス
/// </summary>
class WeaponShot : public WeaponBase
{
public:
	WeaponShot(CSpriteImage* image);
	~WeaponShot();
	void Start() override;
	void Update() override;
	void Draw() override;

private:
	Map* objMap;
	VECTOR2 velocity;
	VECTOR2 PrevVelocity;
	VECTOR2 normal;
	VECTOR2 reflect;
	int BounceCount;
	int mg;

	void updateNormal();
	void updateDamage();
	void updateDead();
};