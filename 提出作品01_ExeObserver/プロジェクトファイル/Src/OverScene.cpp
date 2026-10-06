#include "OverScene.h"
#include "OverDraw.h"
#include "ResultScore.h"
#include "AudioManager.h"
#include "DataCarrier.h"

OverScene::OverScene()
{
	Instantiate<OverDraw>();	  	 // オーバーシーンの描画クラス
	Instantiate<ResultScore>();
}

OverScene::~OverScene()
{
}

void OverScene::Update()
{
	if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_T))
	{
		//ObjectManager::FindGameObject<DataCarrier>()->SetScriptName("");	 // 最初のスクリプトから開始
		//ObjectManager::FindGameObject<DataCarrier>()->ClearScore();		 // スコアをクリヤーする
		SceneManager::ChangeScene("TitleScene");
	}
	else if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_RETURN))
	{
		PostQuitMessage(0);
	}
}

void OverScene::Draw()
{
}
