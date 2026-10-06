#pragma once
#include "EnemyBase.h"
#include "Animator.h"
#include "Player.h"
#include "Executor.h"

class EnemyGolem : public EnemyBase {
public:
	EnemyGolem();
	~EnemyGolem();
	void Update() override;
	SphereCollider Collider() override;

private:
	void updateNormal();
	void updateNormalIdle();
	void updateNormalWalk();
	void updateNormalReach();
	void updateNormalAttack();
	void updateDamage();
	void updateDead();

	int aroundID;
	Executor* targetPlayer;

	enum Phase
	{
		phWait = 0,
		phAttack,
		phEnd
	};
	Phase phase;
};