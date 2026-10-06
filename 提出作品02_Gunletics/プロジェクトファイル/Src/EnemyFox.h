#pragma once
#pragma once
#include "EnemyBase.h"
#include "Animator.h"
#include "Collider.h"
#include "Map.h"

/// <summary>
/// 敵　オオカミの敵のクラス
/// </summary>
class EnemyFox : public EnemyBase
{
public:
	EnemyFox(CSpriteImage* image);
	~EnemyFox();
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