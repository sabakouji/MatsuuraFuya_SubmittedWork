#pragma once
#include "Object2D.h"
#include "Animator.h"
#include "Collider.h"
#include "WeaponManager.h"
#include "Map.h"

class WeaponGrenade : public WeaponBase
{
public:
	WeaponGrenade(CSpriteImage* image);
	~WeaponGrenade();
	void Start() override;
	void Update() override;
	void Draw() override;
private:
	Map* objMap;
	MapLine* pHitMapLine;
	VECTOR2 velocity;
	VECTOR2 PrevVelocity;
	VECTOR2 reflect;
	VECTOR2 normal;
	VECTOR4 SrcPattern;
	float gravityIncremental;
	float gravity;
	float bomTimer;
	float bomrot;
	bool isBlast;

	void updateNormal();
	void updateBlast();
};