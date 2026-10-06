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
	const int MaxHp = 650;
	const int MaxAtc = 400;
	const int MoveSpeed = 1;
	const int FlashWaitTime = 60;
	const int DeadWaitTime = 180;
	const VECTOR4 SrcPattern = VECTOR4(192, 192, 48, 48);
};

EnemyFox::EnemyFox(CSpriteImage* image) : EnemyBase()
{
	// スプライトとアニメーターの初期化
	CreateSprite(image, SrcPattern);
	col = new Collider(this);
	animator = new Animator(this);
	animator->SetDir(Animator::diLeft);

	objMap = nullptr;
	state = State::stNormal;
	hp = MaxHp;
	atc = MaxAtc;
	wait = 0;
}

EnemyFox::~EnemyFox()
{
	SAFE_DELETE(col);
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
}
// 最初に１回だけ実行する処理
void EnemyFox::Start()
{
	objMap = ObjectManager::FindGameObject<Map>();
}

void EnemyFox::Update()
{
	if (!updateEnabled)
	{
		return;
	}

	velocity = VECTOR2(0, 0);

	switch (state) {
	case State::stFlash:
		if (--wait <= 0) {
			state = stNormal;
			animator->ResetFlash();
		}
		updateNormal();
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

	animator->Update();	   // アニメーターの更新

	transform.position += velocity;

}

void EnemyFox::Draw()
{
	Object2D::Draw();
}

// 通常時の更新処理
void EnemyFox::updateNormal()
{
	MapLine* pHitmapline = nullptr;
	bool bRet;

	// 自由移動の処理
	if (animator->GetDir() == Animator::diRight)
	{
		velocity.x = MoveSpeed;
	}
	else {
		velocity.x = -MoveSpeed;
	}

	// マップ線との接触判定と適切な位置への移動
	bRet = objMap->isCollisionMoveMap(this, velocity, pHitmapline);
	if (!bRet || (pHitmapline && pHitmapline->Normal.y > -0.98f))
	{
		// マップ線の端に来たので左右反転
		if (animator->GetDir() == Animator::diRight)
		{
			animator->SetDir(Animator::diLeft);
			velocity.x = -MoveSpeed;
			velocity.y = 0;
		}
		else {
			animator->SetDir(Animator::diRight);
			velocity.x = MoveSpeed;
			velocity.y = 0;
		}
	}

	HitCheck();			// あたり判定
}

// ダメージ時の更新処理
void EnemyFox::updateDamage()
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
}
// 死亡時の更新処理
void EnemyFox::updateDead()
{
	if (--wait <= 0)
	{
		ObjectManager::FindGameObject<DataCarrier>()->AddScore(MaxHp);  // スコアに加算
		DestroyMe();		 // 自分を削除します
	}
}
