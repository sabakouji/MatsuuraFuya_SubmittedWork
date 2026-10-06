// 7行目 [ClearScene] コンストラクタ
// 17行目 [~ClearScene] デストラクタ
// 19行目 [Update] 更新処理

#include "ClearScene.h"
#include "AudioManager.h"
#include "ClearDraw.h"
#include "DataCarrier.h"
#include "ResultScore.h"

ClearScene::ClearScene() {
  Instantiate<ResultScore>();

  DataCarrier *dc = ObjectManager::FindGameObject<DataCarrier>();
  if (dc) {
    dc->CompleteCurrentStage();
  }

  clearimage = new CSpriteImage("Data/Image/StageSelect_BackGround.png");
  sprite = new CSprite();
}

ClearScene::~ClearScene() {
  SAFE_DELETE(clearimage);
  SAFE_DELETE(sprite);
}

void ClearScene::Update() {
  if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_T)) {
    ObjectManager::FindGameObject<DataCarrier>()->SetScriptName("");
    ObjectManager::FindGameObject<DataCarrier>()->ClearScore();
    ObjectManager::FindGameObject<DataCarrier>()->ClearScreenshots();
    SceneManager::ChangeScene("TitleScene");
  } else if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_RETURN)) {
    ObjectManager::FindGameObject<DataCarrier>()->SetScriptName("");
    ObjectManager::FindGameObject<DataCarrier>()->ClearScore();

    // Clear screenshots when leaving ClearScene
    ObjectManager::FindGameObject<DataCarrier>()->ClearScreenshots();

    SceneManager::ChangeScene(
        "StageSelectScene", false); // false = Don't push ClearScene to history
  }
}

void ClearScene::Draw() {
  sprite->Draw(clearimage, 0.0f, 0.0f, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}
