#pragma once
#include "WeaponBase.h"
#include "Animator.h"


class WeaponLaser : public WeaponBase {
public:
	WeaponLaser();
	~WeaponLaser();
	void Update() override;
	void Start() override;
	void Draw() override;

private:
	float farDistance;
};