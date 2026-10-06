#pragma once
#include "EnemyBase.h"
#include "Animator.h"
#include "Collider.h"
#include "Map.h"

/// <summary>
/// “G@‚ª‚Ü‚Ì“G‚ÌƒNƒ‰ƒX
/// </summary>
class EnemyTord : public EnemyBase
{
public:
	EnemyTord(CSpriteImage* image);
	~EnemyTord();
	void Start() override;
	void Update() override;
	void Draw() override;
private:
	Map* objMap;
	VECTOR2 velocity;
	int wait;

	void updateNormal();
	void updateDamage();
	void updateDead();

};