#pragma once
#include "ItemManager.h"
#include "Animator.h"
#include "Collider.h"

/// <summary>
/// アイテムのクラス
/// </summary>
class ItemManager;
class Player;
class Item : public ItemBase
{
public:
	Item(CSpriteImage* image);
	~Item();
	void Start() override;
	void Update() override;
	void Draw() override;

	void SetPos(VECTOR2 pos) { transform.position = pos; }
	void SetKind(ItemKind kd) { kind = kd; }
	void SetEvtOd(int n) { evtOd = n; }
	void TakeItem(Player* pc);
	int EvtOd() { return evtOd; }

private:
	Animator* animator;
	Collider* col;
	VECTOR2 velocity;
	ItemKind  kind;
	int evtOd;

	/// <summary>
	/// PCとのあたり判定
	/// PCに、アイテム取得の処理を行う
	/// </summary>
	/// <returns>当たったらtrue、当たらなかったらfalse</returns>
	bool hitCheck();
};