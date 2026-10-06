#include "GameMain.h"
#include "Sprite.h"
#include "Collider.h"
#include "Player.h"
#include "EnemyBase.h"
#include "EffectManager.h"
#include "ObjectManager.h"
#include "AudioManager.h"
#include "Map.h"
#include "WeaponShot.h"
#include <math.h>

namespace {
	const int MoveSpeed = 40;
	const int MaxAtc = 300;
	const int BaseBounce = 9;
	const VECTOR4 SrcPattern = VECTOR4(0, 0, 51, 5);
	const int MAXMG = 10;
};

WeaponShot::WeaponShot(CSpriteImage* image)	: WeaponBase()
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
	BounceCount = BaseBounce;
	mg = MAXMG;

	AudioManager::Audio("SeShot")->Play();
}

WeaponShot::~WeaponShot()
{
	SAFE_DELETE(col);
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
}

void WeaponShot::Start()
{
	objMap = ObjectManager::FindGameObject<Map>();

	mg -= 1;

	float rad = transform.rotation * Deg2Rad;

	velocity.x = cosf(rad) * MoveSpeed;
	velocity.y = sinf(rad) * MoveSpeed;
}

void WeaponShot::Update()
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

void WeaponShot::Draw()
{
	Object2D::Draw();
}

// 通常時の更新処理
void WeaponShot::updateNormal()
{
	VECTOR2 scroll = GameDevice()->Scroll;

	if (HitCheck())	 // あたり判定
	{
		DestroyMe();	// 当たっていたら自分を削除します
	}

	MapLine* pHitMapLine = nullptr;
	PrevVelocity = velocity;

	if(objMap && objMap->isCollisionMoveMap(this, velocity, pHitMapLine))
	{
		if (BounceCount > 0)
		{
			normal = pHitMapLine->Normal;
			float dot = PrevVelocity.x * normal.x + PrevVelocity.y * normal.y;

			reflect.x = PrevVelocity.x - 2.0f * dot * normal.x;
			reflect.y = PrevVelocity.y - 2.0f * dot * normal.y;

			BounceCount = BounceCount - 1;
			velocity = reflect;
			float reflectangle = atan2f(reflect.y, reflect.x);
			float reflectDeg = reflectangle * (180.0f / 3.14159265f);
			transform.rotation = reflectDeg;
		}
	}
	if (BounceCount <= 0)
	{
		DestroyMe();
	}
	if (pHitMapLine)
	{
		transform.position += velocity;
	}
}

void WeaponShot::updateDamage()
{
}

void WeaponShot::updateDead()
{
}