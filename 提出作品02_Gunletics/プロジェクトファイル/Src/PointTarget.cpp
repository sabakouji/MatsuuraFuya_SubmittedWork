#include "PointTarget.h"
#include "Sprite.h"
#include "DataCarrier.h"
#include "Player.h"
#include "Collider.h"
#include "EnemyManager.h"
#include "WeaponManager.h"
#include "EffectManager.h"
#include "Map.h"

namespace
{
	const int MAXHP = 1;
	const int FlashWaitTime = 60;
	const int DeadWaitTime = 180;
	const VECTOR4 SrcPattern = VECTOR4(0, 155, 27, 42);
	const int point = 1000;
}

PointTarget::PointTarget(CSpriteImage* image)
{
	CreateSprite(image, SrcPattern);
	col = new Collider(this);
	animator = new Animator(this);

	state = State::stNormal;
	hp = MAXHP;
}

PointTarget::~PointTarget()
{
	SAFE_DELETE(col);
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
}

void PointTarget::Update()
{
	if (!updateEnabled)
	{
		return;
	}

	velocity = VECTOR2(0, 0);

	animator->SetAnimationRange(0, 2, 0, true);

	if (state == stNormal)
	{
		animator->SetWaitTime(6);
	}
	else if(state == stDamage)
	{
		animator->SetWaitTime(5);
	}

	switch (state) {
	case State::stFlash:
		if (--wait <= 0) {
			state = stNormal;
			animator->ResetFlash();
		}
		break;
	case State::stNormal:
		updateNormal();
		break;
	case State::stDamage:
		updateDamage();
		break;
	case State::stDead:
		updateDead();
		break;
	}

	animator->Update();
}

void PointTarget::Draw()
{
	Object2D::Draw();
}

void PointTarget::updateNormal()
{
	HitCheck();
}

void PointTarget::updateDamage()
{
	hp -= beDamaged;	// ＰＣや武器から受けるダメージを計算
	beDamaged = 0;
	if (hp <= 0)
	{
		wait = DeadWaitTime;
		state = State::stDead;		   // 死亡処理へ
		animator->SetFlash();
		hp = 0;
	}
	else {
		wait = FlashWaitTime;
		state = State::stFlash;	   	 	// フラッシュ処理へ
		animator->SetFlash();
	}
	hp -= beDamaged;
	beDamaged = 0;

	if (hp <= 0)
	{
		wait = DeadWaitTime;
		state = State::stDead;
		animator->SetFlash();
		hp = 0;
	}
	else {
		wait = FlashWaitTime;
		state = State::stFlash;
		animator->SetFlash();
	}
}

void PointTarget::updateDead()
{
	ObjectManager::FindGameObject<DataCarrier>()->AddScore(point);
	DestroyMe();
}