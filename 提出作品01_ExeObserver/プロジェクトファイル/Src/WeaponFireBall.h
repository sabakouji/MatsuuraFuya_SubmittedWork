#pragma once
#include "WeaponBase.h"
#include "Animator.h"


class WeaponFireBall : public WeaponBase {
public:
	WeaponFireBall();
	~WeaponFireBall();
	void Update() override;
	void Start() override;
	void Draw() override;

private:
	std::list<EffectFire> efList;
	float farDistance;
};