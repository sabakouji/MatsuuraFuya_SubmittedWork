#pragma once
#include "Object2D.h"
#include "Animator.h"
#include "Collider.h"
#include "WeaponManager.h"
#include "Map.h"

/// <summary>
/// 武器ショットのクラス
/// </summary>
class EnemyWeapon : public WeaponBase
{
public:
	EnemyWeapon(CSpriteImage* image);
	~EnemyWeapon();
	void Start() override;
	void Update() override;
	void Draw() override;

private:
	Map* objMap;
	VECTOR2 velocity;

	void updateNormal();
	void updateDamage();
	void updateDead();
};