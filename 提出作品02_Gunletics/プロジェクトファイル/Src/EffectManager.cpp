#include "EffectManager.h"
#include "GameMain.h"
#include "Sprite.h"


EffectManager::EffectManager(CSpriteImage* inimage)
{
	ObjectManager::SetVisible(this, false);		// EffectManager自体は表示はしない
	image = inimage;
}

EffectManager::~EffectManager()
{
}

// 最初に1回だけ行う処理
void EffectManager::Start()
{

}

// 毎回行う更新処理
void EffectManager::Update()
{

}

// 発生中の効果オブジェクトを全て削除する
void EffectManager::DestroyEffect()
{
	std::list<EffectBase*> objlist = ObjectManager::FindGameObjects<EffectBase>();
	for (EffectBase*& el : objlist)
	{
		if (el != this)	   // 自分は削除しない
		{
			ObjectManager::Destroy(el);
		}
	}
}

