#pragma once
#include "EnemyBase.h"
#include "Animator.h"
#include "Collider.h"
#include "Map.h"

/// <summary>
/// “G@ò‚Ì…‚Ì“G‚ÌƒNƒ‰ƒX
/// </summary>
class EnemyWater : public EnemyBase
{
public:
	EnemyWater(CSpriteImage* image);
	~EnemyWater();
	void Start() override;
	void Update() override;
	void Draw() override;
private:
	VECTOR2 velocity;
	int wait;

	void updateNormal();
	void updateDamage();
	void updateDead();

};