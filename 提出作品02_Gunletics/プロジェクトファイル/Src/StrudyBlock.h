#pragma once
#include "EnemyBase.h"
#include "Animator.h"
#include "Collider.h"
#include "Map.h"

class StrudyBlock : public EnemyBase
{
public:
	StrudyBlock(CSpriteImage* image);
	~StrudyBlock();
	void Update() override;
	void Draw() override;
private:
	VECTOR2 velocity;

	void updateNormal();
};