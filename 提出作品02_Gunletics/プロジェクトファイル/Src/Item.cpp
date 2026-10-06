#include "Item.h"
#include "GameMain.h"
#include "Sprite.h"
#include "Collider.h"
#include "Player.h"
#include "AudioManager.h"
#include "Map.h"
#include "Player.h"

Item::Item(CSpriteImage* image)
{
	// スプライトとアニメーター,コリジョンの初期化
	sprite = new CSprite(image);
	col = new Collider(this);
	animator = new Animator(this);
	animator->SetDir((Animator::Dir)0);	// 指定パターン位置のY=0のパターン

	velocity = VECTOR2(0, 0);
	kind = itNone;
	evtOd = 0;
	Itemupdate = true;
}

Item::~Item()
{
	SAFE_DELETE(col);
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
}

void Item::Start()
{
	switch(kind)
	{
	case grenade:
		sprite->SetSrc(0, 0, 52, 27);
		break;
	case penetrate:
		sprite->SetSrc(0, 28, 52, 27);
		break;
	case itDoor:
		sprite->SetSrc(200, 200, 48, 48);
		break;
	}
}

void Item::Update()
{
	if (!Itemupdate)
	{
		return;
	}

	hitCheck();	 // あたり判定

	animator->Update();	   // アニメーターの更新

}

void Item::Draw()
{
	Object2D::Draw();
}

// あたり判定
bool Item::hitCheck()
{
	bool hit = false;

	if (col == nullptr)
		return false;

	Player* pc = nullptr;
	hit = col->Hitcheck<Player>(pc);	 // ＰＣに当たっているとき
	if (hit && pc != nullptr )
	{
		// アイテム取得処理
		TakeItem(pc);
		DestroyMe();  // 自分を削除します
 	}
	return hit;
}

// アイテム取得処理
void Item::TakeItem(Player* pc)
{
	switch (kind)
	{
	case grenade:
		pc->AddWeapon(Weapon_Grenade);
		break;
	case penetrate:
		pc->AddWeapon(Weapon_Penetrate);
		break;
	case itDoor:			  // ドア
		ObjectManager::FindGameObject<Map>()->ChangeMap(evtOd);
		break;
	}

	AudioManager::Audio("SePowerUp")->Play();
}
