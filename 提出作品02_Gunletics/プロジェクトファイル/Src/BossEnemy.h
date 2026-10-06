#pragma once
#include "Animator.h"
#include "Collider.h"
#include "Map.h"
#include "Player.h"
#include "WeaponManager.h"

class WeaponBase;
class BossEnemy : public EnemyBase
{
public:
	BossEnemy(CSpriteImage* image);
	~BossEnemy();
	void Start() override;
	void Update() override;
	void Draw() override;

	VECTOR2 position;

private:
	Map* objMap;
	WeaponManager* weapon;
	Player* PL;
	VECTOR2 velocity;
	VECTOR2 scroll;
	VECTOR2 PlayerPos;
	VECTOR2 playerworldpos;
	VECTOR2 offset;
	VECTOR2 dir;
	VECTOR2 ToPos;
	VECTOR2 enemypos;
	XMMATRIX matScale;
	XMMATRIX matRot;
	XMMATRIX matTrans;
	XMMATRIX worldmat;
	XMVECTOR localVEC;
	XMVECTOR worldVEC;
	XMFLOAT3 enemyShotpos;
	float distance;
	float searchdistance;
	float angleRad;
	float angleDeg;
	int wait;
	int cooldown;
	bool shotcooldown;
	bool shotmode;
	bool rigthside;

	void updateNormal();
	void updateDamage();
	void updateDead();
	void updatedistance();
	void Search();
};
