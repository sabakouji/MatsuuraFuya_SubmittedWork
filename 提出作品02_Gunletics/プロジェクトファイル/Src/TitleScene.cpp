#include "TitleScene.h"
#include "GameMain.h"

TitleScene::TitleScene()
{
	image = new CSpriteImage("Data/Image/my_title.png");
	sprite = new CSprite;
}

TitleScene::~TitleScene()
{
	SAFE_DELETE(image);
	SAFE_DELETE(sprite);
}

void TitleScene::Update()
{
	if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_SPACE) ||
		GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_RETURN))
	{
		SceneManager::ChangeScene("PlayScene");
	}
}

void TitleScene::Draw()
{
	sprite->Draw(image, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}