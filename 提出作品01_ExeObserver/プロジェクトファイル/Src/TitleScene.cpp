#include "TitleScene.h"
#include "AudioManager.h"
#include "DataCarrier.h"
#include "FadeObject.h"
#include "GameMain.h"
#include "SceneManager.h"
#include "TitleBackground.h"
#include "TitleDraw.h"

TitleScene::TitleScene() {
  Instantiate<TitleBackground>();
  Instantiate<TitleDraw>();

  DataCarrier *dc = ObjectManager::FindGameObject<DataCarrier>();
  if (dc)
    dc->ClearScore();

  m_fade = nullptr;
  m_isFading = false;
}

TitleScene::~TitleScene() {}

void TitleScene::Update() {
  if (m_isFading)
    return;

  if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_RETURN)) {
    m_isFading = true;
    m_fade = Instantiate<FadeObject>();
    m_fade->StartFadeOut(
        1.0f, []() { SceneManager::ChangeScene("ModeSelectScene"); });
  }
}

void TitleScene::Draw() {}