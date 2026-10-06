#include "PlayScene.h"
#include "DataCarrier.h"
#include "Player.h"
#include "EnemyManager.h"
#include "WeaponManager.h"
#include "EffectManager.h"
#include "ItemManager.h"
#include "Map.h"
#include "DisplayInfo.h"
#include "AudioManager.h"

PlayScene::PlayScene()
{
	imageChar = new CSpriteImage(_T("Data/Image/Player_1_v8.png"));
	imageEnemy = new CSpriteImage(_T("Data/Image/My_Enemy.png"));
	imageSprite = new CSpriteImage(_T("Data/Image/sprite1.png"));
	imageammo = new CSpriteImage(_T("Data/Image/my_ammo.png"));
	imageUI = new CSpriteImage(_T("Data/Image/my_UI.png"));
	imageitem = new CSpriteImage(_T("Data/Image/my_item.png"));

	Instantiate<Player>(imageChar);
	Instantiate<EnemyManager>(imageEnemy);
	Instantiate<Map>();
	Instantiate<WeaponManager>(imageammo);
	Instantiate<EffectManager>(imageSprite);
	Instantiate<ItemManager>(imageitem);
	Instantiate<DisplayInfo>(imageUI);

	AudioManager::Audio("Bgm1")->Play(AUDIO_LOOP);
}

PlayScene::~PlayScene()
{
	SAFE_DELETE(imageChar);
	SAFE_DELETE(imageSprite);
}

void PlayScene::Update()
{
	Player* obj = ObjectManager::FindGameObject<Player>();
	CDirectInput* input = GameDevice()->m_pDI;

	if (input->CheckKey(KD_TRG, DIK_TAB))
	{
		if (!stState)
		{
			stState = true;
			obj->SetStop(true);
			emanager->SetUpdateEnabled(false);
			weapon->SetWeaponUpdateEnabled(false);
			iManager->SetStopItemUpdate(false);
		}
		else if (stState)
		{
			stState = false;
			obj->SetStop(false);
			obj->StopState = false;
			emanager->SetUpdateEnabled(true);
			weapon->SetWeaponUpdateEnabled(true);
			iManager->SetStopItemUpdate(true);
		}
	}
	if (input->CheckKey(KD_TRG, DIK_T))
	{
		if (stState)
		{
			SceneManager::ChangeScene("TitleScene");
		}
	}
}

void PlayScene::Draw()
{
}