#include "GameMain.h"
#include "Sprite.h"
#include "DataCarrier.h"
#include "Player.h"
#include "Collider.h"
#include "EnemyManager.h"
#include "WeaponManager.h"
#include "EffectManager.h"
#include "Map.h"

namespace {
	const int MaxHp = 10000;
	const int MaxAtc = 200;
	const VECTOR4 SrcPattern = VECTOR4(384, 240, 48, 48);
};

EnemyWater::EnemyWater(CSpriteImage* image) : EnemyBase()
{
	// スプライトとアニメーターの初期化
	CreateSprite(image, SrcPattern);
	col = new Collider(this);
	animator = new Animator(this);
	animator->SetDir((Animator::Dir)0);	// 指定パターン位置のY=0のパターン
	animator->SetAlpha(0.8f);

	state = State::stNormal;
	hp = MaxHp;
	atc = MaxAtc;
	wait = 0;
}

EnemyWater::~EnemyWater()
{
	SAFE_DELETE(col);
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
}
// 最初に１回だけ実行する処理
void EnemyWater::Start()
{
}
void EnemyWater::Update()
{
	if (!updateEnabled)
	{
		return;
	}

	velocity = VECTOR2(0, 0);

	switch (state) {
	case State::stFlash:
		state = State::stNormal;
		updateNormal();
		break;
	case State::stNormal:
		updateNormal();
		break;
	case State::stDamage:
		state = State::stNormal;
		break;
	case State::stDead:
		state = State::stNormal;
		break;
	}

	animator->Update();	   // アニメーターの更新

	transform.position += velocity;

}

void EnemyWater::Draw()
{
	Object2D::Draw();
}

// 通常時の更新処理
void EnemyWater::updateNormal()
{
	HitCheck();			// あたり判定
}

// ダメージ時の更新処理
void EnemyWater::updateDamage()
{
}
// 死亡時の更新処理
void EnemyWater::updateDead()
{
}
