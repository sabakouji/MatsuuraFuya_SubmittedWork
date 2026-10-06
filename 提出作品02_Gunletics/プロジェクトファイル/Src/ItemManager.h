#pragma once
#include "Object2D.h"
#include "Animator.h"
#include "ItemBase.h"
#include "Item.h"

/// <summary>
/// アイテム管理クラス
/// </summary>
class ItemManager : public ItemBase
{
public:
	ItemManager(CSpriteImage* image);
	~ItemManager();
	void Start() override;
	void Update() override;

	void SetStopItemUpdate(bool Stop);

	/// <summary>
	/// アイテムの発生処理
	/// </summary>
	/// <typeparam name="C">アイテムクラス名</typeparam>
	/// <param name="pos">発生位置</param>
	/// <param name="rot">発生角度</param>
	/// <param name="kd">アイテム種類</param>
	/// <param name="num">発生アイテム数</param>
	/// <returns>発生できたときはオブジェクト,できないときはnullptr</returns>
	template<class C> C* Spawn(VECTOR2 pos, float rot, ItemKind kd, int num)
	{
		C* obj = Instantiate<C>(image);
		if (obj == nullptr)  return nullptr;
		obj->SetPos(pos);
		obj->SetKind(kd);
		return obj;
	}

	/// <summary>
	/// 発生中のアイテムを全て削除する
	/// </summary>
	void DestroyItem();

private:
	CSpriteImage* image;
	
	bool spawnDone;
};