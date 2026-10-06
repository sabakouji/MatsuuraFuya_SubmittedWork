#pragma once
#include "WeaponBase.h"
#include "Animator.h"
#include "EffectBase.h"

class WeaponFireBall2 : public WeaponBase {
public:
	WeaponFireBall2();
	~WeaponFireBall2();
	void Update() override;
	void Start() override;
	void Draw() override;

private:
	std::list<EffectFire> efList;
	float farDistance;
};