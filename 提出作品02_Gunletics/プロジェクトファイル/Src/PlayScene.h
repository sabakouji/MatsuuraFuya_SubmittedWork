#pragma once
#include "SceneBase.h"

/// <summary>
/// プレイシーンのクラス
/// </summary>
/// 
class ItemManager;
class EnemyManager;
class WeaponManager;
class Player;
class PlayScene : public SceneBase
{
public:
	PlayScene();
	~PlayScene();
	void Update() override;
	void Draw() override;

	bool stState;
private:
	CSpriteImage* imageChar;
	CSpriteImage* imageSprite;
	CSpriteImage* imageEnemy;
	CSpriteImage* imagetarget;
	CSpriteImage* imageammo;
	CSpriteImage* imageUI;
	CSpriteImage* imageitem;
	WeaponManager* weapon;
	EnemyManager* emanager;
	ItemManager* iManager;
	CSprite* sprite;
};