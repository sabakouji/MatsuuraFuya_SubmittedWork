#include "EnemyWeapon.h"
#include "GameMain.h"
#include "Sprite.h"
#include "Collider.h"
#include "Player.h"
#include "EnemyBase.h"
#include "EffectManager.h"
#include "ObjectManager.h"
#include "AudioManager.h"
#include "Map.h"

namespace {
	const int MoveSpeed = 18;
	const int MaxAtc = 100;
	const VECTOR4 SrcPattern = VECTOR4(0, 0, 51, 5);
};

EnemyWeapon::EnemyWeapon(CSpriteImage* image) : WeaponBase()
{
	// スプライトとアニメーター,コリジョンの初期化
	CreateSprite(image, SrcPattern);
	transform.center.x = sprite->GetSrcWidth() / 2;
	transform.center.y = sprite->GetSrcHeight() / 2;

	col = new Collider(this);
	animator = new Animator(this);
	objMap = nullptr;
	state = State::stNormal;
	atc = MaxAtc;

	AudioManager::Audio("SeShot")->Play();
}

EnemyWeapon::~EnemyWeapon()
{
	SAFE_DELETE(col);
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
}

void EnemyWeapon::Start()
{
	objMap = ObjectManager::FindGameObject<Map>();

	float rad = transform.rotation * Deg2Rad;

	velocity.x = cosf(rad) * MoveSpeed;
	velocity.y = sinf(rad) * MoveSpeed;
}

void EnemyWeapon::Update()
{
	if (!WeaponupdateEnabled)
	{
		return;
	}

	switch (state) {
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

void EnemyWeapon::Draw()
{
	Object2D::Draw();
}

// 通常時の更新処理
void EnemyWeapon::updateNormal()
{
	VECTOR2 scroll = GameDevice()->Scroll;

	if (transform.position.x - scroll.x <  -(Sprite()->GetSrcWidth() - transform.center.x) ||
		transform.position.x - scroll.x >	WINDOW_WIDTH + transform.center.x ||
		transform.position.y - scroll.y < -(Sprite()->GetSrcHeight() - transform.center.y) ||
		transform.position.y - scroll.y >(WINDOW_HEIGHT + transform.center.y))	 // 画面外へ出た
	{
		DestroyMe();		 // 自分を削除します
	}
	else {
		if (HitCheck())	 // あたり判定
		{
			DestroyMe();	// 当たっていたら自分を削除します
		}
	}

	MapLine* pHitMapLine = nullptr;

	if (objMap && objMap->isCollisionMoveMap(this, velocity, pHitMapLine))
	{
		DestroyMe();
	}
}

void EnemyWeapon::updateDamage()
{
}

void EnemyWeapon::updateDead()
{
}