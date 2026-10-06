#pragma once
#include "EnemyBase.h"
#include "Animator.h"
#include "Collider.h"
#include "Map.h"

class PointTarget : public EnemyBase
{
public:
	PointTarget(CSpriteImage* image);
	~PointTarget();
	void Update() override;
	void Draw() override;
private:
	VECTOR2 velocity;
	int hp;
	int wait;

	void updateNormal();
	void updateDamage();
	void updateDead();
};