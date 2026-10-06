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
	const int MaxHp = 3000;
	const int MaxAtc = 100;
	const int MoveSpeed = 1;
	const int FlashWaitTime = 30;
	const int DeadWaitTime = 120;
	const int Basecolldown = 120;
	const VECTOR4 SrcPattern = VECTOR4(0, 0, 102, 65);
	const int point = 500;
};

BossEnemy::BossEnemy(CSpriteImage* image) : EnemyBase()
{
	// スプライトとアニメーターの初期化
	CreateSprite(image, SrcPattern);
	col = new Collider(this);
	animator = new Animator(this);

	objMap = nullptr;
	state = State::stNormal;
	hp = MaxHp;
	atc = MaxAtc;
	cooldown = Basecolldown;
	searchdistance = 400.0f;
	shotcooldown = false;
	shotmode = false;
	rigthside = false;
}

BossEnemy::~BossEnemy()
{
	SAFE_DELETE(col);
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
}
// 最初に１回だけ実行する処理
void BossEnemy::Start()
{
	objMap = ObjectManager::FindGameObject<Map>();
	weapon = ObjectManager::FindGameObject<WeaponManager>();
	PL = ObjectManager::FindGameObject<Player>();
}

void BossEnemy::Update()
{
	if (!updateEnabled) return;
	if (!PL || !weapon) return;

	if (!shotcooldown)
	{
		cooldown--;
		if (cooldown <= 0)
		{
			shotcooldown = true;
		}
	}

	velocity = VECTOR2(0, 0);

	PlayerPos = PL->Getposition();

	switch (state) {
	case State::stFlash:
		if (--wait <= 0) {
			state = stNormal;
			animator->ResetFlash();
		}
		updateNormal();
		updatedistance();
		Search();
		break;
	case State::stNormal:
		updateNormal();
		updatedistance();
		Search();
		break;
	case State::stDamage:
		updateDamage();
		updatedistance();
		Search();
		break;
	case State::stDead:
		updateDead();
		break;
	}

	transform.position += velocity;

	animator->Update();
}

void BossEnemy::Draw()
{
	Object2D::Draw();
}

void BossEnemy::updatedistance()
{
	scroll = GameDevice()->Scroll;

	playerworldpos.x = PlayerPos.x + scroll.x;
	playerworldpos.y = PlayerPos.y + scroll.y;

	enemypos = transform.position + scroll;

	ToPos = playerworldpos - enemypos;

	distance = sqrtf(ToPos.x * ToPos.x + ToPos.y * ToPos.y);
}

void BossEnemy::Search()
{
	scroll = GameDevice()->Scroll;

	if (distance <= searchdistance)
	{
		shotmode = true;

		if (playerworldpos.x > enemypos.x)
		{
			rigthside = true;
			animator->SetAnimationRange(1, 1, 0, false);
		}
		else
		{
			rigthside = false;
			animator->SetAnimationRange(0, 1, 0, false);
		}

		if (shotcooldown)
		{
			offset.x = -transform.center.x + sprite->GetDestWidth() / 2.0f;
			offset.y = -transform.center.y + sprite->GetDestHeight() / 2.0f;

			matScale = XMMatrixScaling(transform.scale.x, transform.scale.y, 1.0f);
			matRot = XMMatrixRotationZ(XMConvertToRadians(transform.rotation));
			matTrans = XMMatrixTranslation(transform.position.x, transform.position.y, 1.0f);

			worldmat = matScale * matRot * matTrans;

			localVEC = XMVectorSet(offset.x, offset.y, 0.0f, 1.0f);
			worldVEC = XMVector4Transform(localVEC, worldmat);

			XMStoreFloat3(&enemyShotpos, worldVEC);

			dir = PlayerPos - VECTOR2(enemyShotpos.x, enemyShotpos.y + 12.0f);
			angleRad = atan2f(dir.y, dir.x);
			angleDeg = angleRad * (180 / 3.14159265f);

			weapon->Spawn<EnemyWeapon>(VECTOR2{ enemyShotpos.x,enemyShotpos.y }, angleDeg, WeaponBase::eENM);

			shotcooldown = false;
			cooldown = Basecolldown;
		}
	}
	else
	{
		shotmode = false;
	}
}

// 通常時の更新処理
void BossEnemy::updateNormal()
{
	MapLine* pHitmapline = nullptr;
	bool bRet;

	// 自由移動の処理
	if (shotmode == false)
	{
		if (rigthside)
		{
			velocity.x = MoveSpeed;
		}
		else
		{
			velocity.x = -MoveSpeed;
		}
	}

	// マップ線との接触判定と適切な位置への移動
	bRet = objMap->isCollisionMoveMap(this, velocity, pHitmapline);
	if (!bRet || (pHitmapline && pHitmapline->Normal.y > -0.98f))
	{
		// マップ線の端に来たので左右反転
		if (animator->GetDir() == Animator::diRight)
		{
			rigthside = false;
			animator->SetDir(Animator::diLeft);
			animator->SetAnimationRange(0, 1, 0, false);
			velocity.x = -MoveSpeed;
			velocity.y = 0;
		}
		else
		{
			rigthside = true;
			animator->SetDir(Animator::diRight);
			animator->SetAnimationRange(1, 1, 0, false);
			velocity.x = MoveSpeed;
			velocity.y = 0;
		}
	}

	HitCheck();			// あたり判定
}

// ダメージ時の更新処理
void BossEnemy::updateDamage()
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
void BossEnemy::updateDead()
{
	if (--wait <= 0)
	{
		ObjectManager::FindGameObject<DataCarrier>()->AddScore(point);  // スコアに加算
		DestroyMe();		 // 自分を削除します
	}
}