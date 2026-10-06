#include "OverScene.h"
#include "GameMain.h"
#include "AudioManager.h"

OverScene::OverScene()
{
	image = new CSpriteImage("Data/Image/my_gameover.png");
	sprite = new CSprite;

	AudioManager::Audio("Bgm1")->Stop();

}

OverScene::~OverScene()
{
	SAFE_DELETE(image);
	SAFE_DELETE(sprite);
}

void OverScene::Update()
{
	if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_C))
	{
		// ゲームオーバー時のコンティニューで、現ステージをやり直す
		SceneManager::ChangeScene("PlayScene");
	}
}

void OverScene::Draw()
{
	sprite->Draw(image, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}