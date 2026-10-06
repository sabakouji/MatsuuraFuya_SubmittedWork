#include "EffectBom.h"
#include "GameMain.h"
#include "Sprite.h"
#include "Collider.h"
#include "Player.h"
#include "AudioManager.h"
#include "Map.h"

namespace {
	const VECTOR4 SrcPattern = VECTOR4(144, 480, 64, 64);
	const int PatternNum = 7;
};

EffectBom::EffectBom(CSpriteImage* image) : EffectBase()
{
	// スプライトとアニメーター,コリジョンの初期化
	CreateSprite(image, SrcPattern);
	animator = new Animator(this);
	animator->SetAnimNum(PatternNum);

	AudioManager::Audio("SeNitro")->Play();

}

EffectBom::~EffectBom()
{
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
}


void EffectBom::Update()
{
	animator->Update();	   // アニメーターの更新

	if (animator->IsFinished())	  // アニメーションの終了
	{
		DestroyMe();		 // 自分を削除します
	}
}

void EffectBom::Draw()
{
	Object2D::Draw();
}

