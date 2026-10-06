#pragma once

#include "Object3D.h"
#include "Animator.h"
#include "WeaponManager.h"

class Player : public Object3D
{
public:
	Player();
	~Player();
	void Update() override;
	void Draw() override;

	SphereCollider Collider() override;
	void AddDamage(float damage, VECTOR3 pPos);
	int Number() { return number; }
	int HitPoint() { return hitPoint; }
	float HpdivMax();
	void SetMaxHp();

private:
	float   speedY;
	VECTOR3 velocity;
	int     number;
	int     hitPoint;
	int     attackPoint;
	float   flashTimer;
	WeaponSword* swordObj;
	WeaponGun* gunObj;

	enum AnimID 
	{
		aIdle = 0,
		aRun = 1,
		aDead,
		aAttack1,
		aAttack2,
		aAttack3,
	};

	enum State 
	{
		stNormal = 0,
		stDamage,
		stDead,
		stFlash
	};

	const enum AtcState 
	{
		atIdle = 0,
		atWalk,
		atAttack,
	};

	State state;
	AtcState atcstate;
	void updateNormal();
	void updateNormalWalk();
	void updateNormalAttack();
	void updateDamage();
	void updateDead();
};