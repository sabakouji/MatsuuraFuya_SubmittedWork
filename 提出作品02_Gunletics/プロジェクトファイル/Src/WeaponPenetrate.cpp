#include "WeaponPenetrate.h"
#include "GameMain.h"
#include "Sprite.h"
#include "Collider.h"
#include "Player.h"
#include "EnemyBase.h"
#include "EffectManager.h"
#include "ObjectManager.h"
#include "AudioManager.h"
#include "Map.h"

namespace
{
	const int MoveSpeed = 24;
	const int MaxAtc = 300;
	const VECTOR4 SrcPattern = VECTOR4(0, 6, 51, 5);
	const int Maxmgsize = 10;
	const int ammosize = 1;
}

WeaponPenetrate::WeaponPenetrate(CSpriteImage* image) : WeaponBase()
{
	CreateSprite(image, SrcPattern);
	transform.center.x = sprite->GetSrcWidth() / 2;
	transform.center.y = sprite->GetSrcHeight() / 2;

	col = new Collider(this);
	animator = new Animator(this);
	objMap = nullptr;
	state = State::stNormal;
	atc = MaxAtc;
	AudioManager::Audio("SeShot")->Play();

	mgsize = Maxmgsize;
}

WeaponPenetrate::~WeaponPenetrate()
{
	SAFE_DELETE(col);
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
}

void WeaponPenetrate::Start()
{
	objMap = ObjectManager::FindGameObject<Map>();

	float rad = transform.rotation * Deg2Rad;

	velocity.x = cosf(rad) * MoveSpeed;
	velocity.y = sinf(rad) * MoveSpeed;
}

void WeaponPenetrate::Update()
{
	if (!WeaponupdateEnabled)
	{
		return;
	}

	switch (state) {
	case State::stNormal:
		updateNormal();
		break;
	}

	animator->Update();

	transform.position += velocity;
}

void WeaponPenetrate::Draw()
{
	Object2D::Draw();
}

void WeaponPenetrate::updateNormal()
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

	if (objMap && objMap->isCollisionMoveMap(this, velocity, pHitMapLine))
	{
		DestroyMe();
	}
	HitCheck();
}