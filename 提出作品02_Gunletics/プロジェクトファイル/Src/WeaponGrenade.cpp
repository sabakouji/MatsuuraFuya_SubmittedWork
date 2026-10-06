#include "GameMain.h"
#include "Sprite.h"
#include "Collider.h"
#include "Player.h"
#include "EnemyBase.h"
#include "EffectManager.h"
#include "ObjectManager.h"
#include "AudioManager.h"
#include "Map.h"
#include "WeaponGrenade.h"

namespace
{
	const int MoveSpeed = 8;
	const int MaxAtc = 900;
	const float detonationTime = 120.0f;
	const VECTOR4 GrenadePattern = VECTOR4(0, 11, 36, 30);
	const int PatternNum = 7;
}

WeaponGrenade::WeaponGrenade(CSpriteImage* image) : WeaponBase()
{
	SrcPattern = GrenadePattern;
	CreateSprite(image, SrcPattern);
	transform.center.x = sprite->GetSrcWidth() / 2;
	transform.center.y = sprite->GetSrcHeight() / 2;

	col = new Collider(this);
	animator = new Animator(this);
	objMap = nullptr;
	state = State::stNormal;
	atc = MaxAtc;

	animator->SetWaitTime(20);

	AudioManager::Audio("SeShot")->Play();

	gravity = 0.0f;
	gravityIncremental = 0.02f;
	bomTimer = 0.0f;
	bomrot = 5.0f;

	isBlast = false;
}

WeaponGrenade::~WeaponGrenade()
{
	SAFE_DELETE(col);
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
}

void WeaponGrenade::Start()
{
	objMap = ObjectManager::FindGameObject<Map>();

	float rad = transform.rotation * Deg2Rad;

	velocity.x = cosf(rad) * MoveSpeed;
	velocity.y = sinf(rad) * MoveSpeed;
}

void WeaponGrenade::Update()
{
	if (!WeaponupdateEnabled)
	{
		return;
	}

	if (!isBlast)
	{
		bomTimer++;
		gravity += gravityIncremental;
		velocity.y += gravity;

		animator->SetAnimationRange(0, 2, 0, false);
		transform.rotation += bomrot;
	}

	switch (state) {
	case State::stNormal:
		updateNormal();
		break;
	}

	animator->Update();
	transform.position += velocity;

	if (isBlast)
	{
		if (animator->IsFinished())
		{
			DestroyMe();
		}
	}
}

void WeaponGrenade::Draw()
{
	Object2D::Draw();
}

void WeaponGrenade::updateNormal()
{
	VECTOR2 scroll = GameDevice()->Scroll;

	if (transform.position.x - scroll.x <  -(Sprite()->GetSrcWidth() - transform.center.x) ||
		transform.position.x - scroll.x >	WINDOW_WIDTH + transform.center.x ||
		transform.position.y - scroll.y < -(Sprite()->GetSrcHeight() - transform.center.y) ||
		transform.position.y - scroll.y >(WINDOW_HEIGHT + transform.center.y))
	{
		DestroyMe();
	}

	pHitMapLine = nullptr;
	PrevVelocity = velocity;

	if (objMap && objMap->isCollisionMoveMap(this, velocity, pHitMapLine))
	{
		PrevVelocity = PrevVelocity / 2;
		gravity = 0;
		normal = pHitMapLine->Normal;

		float dot = PrevVelocity.x * normal.x + PrevVelocity.y * normal.y;

		reflect.x = PrevVelocity.x - 2.0f * dot * normal.x;
		reflect.y = PrevVelocity.y - 2.0f * dot * normal.y;

		reflect.x *= 0.8f;
		reflect.y *= 0.5f;

		velocity = reflect;
		bomrot = bomrot - 1.0f;
	}
	if (pHitMapLine)
	{
		transform.position += velocity;
	}
	if (bomTimer >= detonationTime)
	{
		updateBlast();
		isBlast = true;
	}

	if (HitCheck())
	{
		updateBlast();
		isBlast = true;
	}
}

void WeaponGrenade::updateBlast()
{
	velocity = VECTOR2(0, 0);

	sprite->SetSrc(0, 41, 64, 64);

	transform.center.x = sprite->GetSrcWidth() / 2;
	transform.center.y = sprite->GetSrcHeight() / 2;
	transform.scale = VECTOR2(1.5f, 1.5f);

	animator->SetWaitTime(5);
	animator->SetAnimationRange(0, 6, 0, false);
}