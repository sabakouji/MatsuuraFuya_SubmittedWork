#include "GameMain.h"
#include "Sprite.h"
#include "WeaponManager.h"


WeaponManager::WeaponManager(CSpriteImage* inimage)
{
	ObjectManager::SetVisible(this, false);		// WeaponManager自体は表示しない
	image = inimage;
}

WeaponManager::~WeaponManager()
{
}

// 最初に1回だけ行う処理
void WeaponManager::Start()
{
}

// 毎回行う更新処理
void WeaponManager::Update()
{
	
}

// 発生中の武器オブジェクトを全て削除する
void WeaponManager::DestroyWeapon()
{
	std::list<WeaponBase*> objlist = ObjectManager::FindGameObjects<WeaponBase>();
	for (WeaponBase*& wl : objlist)
	{
		if (wl != this)	   // 自分は削除しない
		{
			ObjectManager::Destroy(wl);
		}
	}
}

void WeaponManager::SetWeaponUpdateEnabled(bool Wenabled)
{
	std::list<Object2D*> objects = ObjectManager::FindGameObjects<Object2D>();

	for (Object2D* obj : objects)
	{
		WeaponBase* WeponB = dynamic_cast<WeaponBase*>(obj);
		if (WeponB && WeponB != this)
		{
			WeponB->SetWeaponUpdateEnabled(Wenabled);
		}
	}
}