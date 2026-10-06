#include "StrudyBlock.h"
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
	const VECTOR4 SrcPattern = VECTOR4(0, 197, 53, 53);
}

StrudyBlock::StrudyBlock(CSpriteImage* image)
{
	CreateSprite(image, SrcPattern);
	col = new Collider(this);
	animator = new Animator(this);
}

StrudyBlock::~StrudyBlock()
{
	SAFE_DELETE(col);
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
}

void StrudyBlock::Update()
{
	if (!updateEnabled)
	{
		return;
	}

	velocity = VECTOR2(0, 0);

	updateNormal();
}

void StrudyBlock::Draw()
{
	Object2D::Draw();
}

void StrudyBlock::updateNormal()
{

}