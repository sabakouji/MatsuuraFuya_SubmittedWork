#include "ItemManager.h"
#include "GameMain.h"
#include "Sprite.h"
#include "Map.h"


ItemManager::ItemManager(CSpriteImage* inimage)
{
	ObjectManager::SetVisible(this, false);		// ItemManager自体は表示はしない
	image = inimage;
	spawnDone = false;
}

ItemManager::~ItemManager()
{	
}

// 最初に1回だけ行う処理
void ItemManager::Start()
{
}

// 毎回行う更新処理
void ItemManager::Update()
{
	if (spawnDone) return;	  // アイテムが出現済みのときは、終了

	Map* objmap = ObjectManager::FindGameObject<Map>();
	VECTOR2 pos;
	int evOd;
	int nNext = 0;

	// イベントマップを探索し、アイテムの出現位置を設定する
	while (nNext != -1)
	{
		// イベントマップ　 ( EvtID:2　アイテムの出現位置,     　EvtNo:0x10　救急箱  )
		if (objmap->SearchEvt(nNext, 2, 0x10, evOd, pos, nNext))
		{
			Item* obj = Instantiate<Item>(image);
			obj->SetPosition(pos + obj->Transform().center);   // posはチップの左上座標なので中心点を足す
			obj->SetKind(itRescue);
			obj->SetEvtOd(evOd);
		}
	}

	// イベントマップを探索し、アイテムの出現位置を設定する
	nNext = 0;
	while (nNext != -1)
	{
		// イベントマップ　 ( EvtID:2　アイテムの出現位置,     　EvtNo:0x20　ドア  )
		if (objmap->SearchEvt(nNext, 2, 0x20, evOd, pos, nNext))
		{
			Item* obj = Instantiate<Item>(image);
			obj->SetPosition(pos + obj->Transform().center);	// posはチップの左上座標なので中心点を足す
			obj->SetKind(itDoor);
			obj->SetEvtOd(evOd);
		}
	}

	nNext = 0;
	while (nNext != -1)
	{
		if (objmap->SearchEvt(nNext, 2, 0x50, evOd, pos, nNext))
		{
			Item* obj = Instantiate<Item>(image);
			obj->SetPosition(pos + obj->Transform().center);
			obj->SetKind(grenade);
			obj->SetEvtOd(evOd);
		}
	}

	nNext = 0;
	while (nNext != -1)
	{
		if (objmap->SearchEvt(nNext, 2, 0x05, evOd, pos, nNext))
		{
			Item* obj = Instantiate<Item>(image);
			obj->SetPosition(pos + obj->Transform().center);
			obj->SetKind(penetrate);
			obj->SetEvtOd(evOd);
		}
	}
	spawnDone = true;	   // アイテム出現済みにする
}

void ItemManager::SetStopItemUpdate(bool Stop)
{
	std::list<Object2D*> objects = ObjectManager::FindGameObjects<Object2D>();

	for (Object2D* obj : objects)
	{
		ItemBase* itembase = dynamic_cast<ItemBase*>(obj);
		if (itembase && itembase != this)
		{
			itembase->StopItem(Stop);
		}
	}
}

// 発生中のアイテムオブジェクトを全て削除する
void ItemManager::DestroyItem()
{
	std::list<ItemBase*> objlist = ObjectManager::FindGameObjects<ItemBase>();
	for (ItemBase*& el : objlist)
	{
		if (el != this)	   // 自分は削除しない
		{
			ObjectManager::Destroy(el);
		}
	}
	spawnDone = false;		 // アイテムの発生フラグをリセットする
}