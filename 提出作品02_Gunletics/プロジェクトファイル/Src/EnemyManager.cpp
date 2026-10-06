#include "Sprite.h"
#include "EnemyManager.h"
#include "EnemyBase.h"
#include "Map.h"

namespace {

};

EnemyManager::EnemyManager(CSpriteImage* inimage)
{
	ObjectManager::SetVisible(this, false);		// EnemyManager自体は表示しない

	image = inimage;
	count = 0;
	spawnDone = false;
}

EnemyManager::~EnemyManager()
{
}

void EnemyManager::Update()
{
	if (spawnDone) return;

	Map* objmap = ObjectManager::FindGameObject<Map>();
	VECTOR2 pos;
	int evOd;
	int nNext = 0;

	// イベントマップを探索し、敵の出現位置を設定する
	while (nNext != -1)
	{
		// イベントマップ　 ( EvtID:3　敵の出現位置,     　EvtNo:0x01　泉の水の敵  )
		if (objmap->SearchEvt(nNext, 3, 0x01, evOd, pos, nNext))
		{
			EnemyWater* obj = Instantiate<EnemyWater>(image);
			obj->SetPosition( pos + obj->Transform().center);	  // posはチップの左上座標なので中心点を足す
		}
	}

	// イベントマップを探索し、敵の出現位置を設定する
	nNext = 0;
	while (nNext != -1)
	{
		// イベントマップ　 ( EvtID:3　敵の出現位置,     　EvtNo:0x02　がまの敵  )
		if (objmap->SearchEvt(nNext, 3, 0x02, evOd, pos, nNext))
		{
			EnemyTord* obj = Instantiate<EnemyTord>(image);
			obj->SetPosition( pos + obj->Transform().center);	  // posはチップの左上座標なので中心点を足す
		}
	}

	// イベントマップ　 ( EvtID:3　敵の出現位置,     　EvtNo:0x04　オオカミの敵  )
	nNext = 0;
	while (nNext != -1)
	{
		// イベントマップ　 ( EvtID:3　敵の出現位置,     　EvtNo:0x04　オオカミの敵  )
		if (objmap->SearchEvt(nNext, 3, 0x04, evOd, pos, nNext))
		{
			EnemyWolf* obj = Instantiate<EnemyWolf>(image);
			obj->SetPosition( pos + obj->Transform().center);	  // posはチップの左上座標なので中心点を足す
		}
	}

	nNext = 0;
	while (nNext != -1)
	{
		if (objmap->SearchEvt(nNext, 3, 0x10, evOd, pos, nNext))
		{
			StrudyBlock* obj = Instantiate<StrudyBlock>(image);
			obj->SetPosition( pos + obj->Transform().center);
		}
	}

	nNext = 0;
	while (nNext != -1)
	{
		if (objmap->SearchEvt(nNext, 3, 0x20, evOd, pos, nNext))
		{
			PointTarget* obj = Instantiate<PointTarget>(image);
			obj->SetPosition(pos + obj->Transform().center);
		}
	}

	nNext = 0;
	while (nNext != -1) {
		if (objmap->SearchEvt(nNext, 3, 0x08, evOd, pos, nNext)) {
			EnemyFox* obj = Instantiate<EnemyFox> (image);
			obj->SetPosition(pos + obj->Transform().center);
		}
	}

	spawnDone = true;
}

// 発生中の敵オブジェクトを全て削除する
void EnemyManager::DestroyEnemy()
{
	std::list<EnemyBase*> objlist = ObjectManager::FindGameObjects<EnemyBase>();
	for (EnemyBase*& el : objlist)
	{
		if (el != this)	   // 自分は削除しない
		{
			ObjectManager::Destroy(el);
		}
	}
	spawnDone = false;		 // 敵の発生フラグをリセットする
}

void EnemyManager::SetUpdateEnabled(bool enabled)
{
	std::list<Object2D*> objects = ObjectManager::FindGameObjects<Object2D>();

	for (Object2D* obj : objects)
	{
		EnemyBase* enemy = dynamic_cast<EnemyBase*>(obj);
		if (enemy && enemy != this)
		{
			enemy->SetUpdateEnabled(enabled);
		}
	}
}