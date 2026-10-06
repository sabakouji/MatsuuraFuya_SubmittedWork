#include "PauseObject.h"
#include "CDrawUtil.h"
#include "GameMain.h"
#include "SceneManager.h"

PauseObject::PauseObject() {}

PauseObject::~PauseObject() {}

void PauseObject::Start() {
  // Draw very late (on top of everything)
  ObjectManager::SetDrawOrder(this, -2000);
}

void PauseObject::Update() {
  // Handle Input for Buttons
  POINT pt;
  GetCursorPos(&pt);
  ScreenToClient(GameDevice()->m_pMain->m_hWnd, &pt);
  bool isClicked = GameDevice()->m_pDI->CheckMouse(KD_TRG, 0); // Left Click

  float sw = (float)WINDOW_WIDTH;
  float sh = (float)WINDOW_HEIGHT;
  float centerX = sw * 0.5f;
  float bottomY = sh - 100.0f;

  // Button Dimensions
  float btnW = 200.0f;
  float btnH = 50.0f;
  float gap = 20.0f;

  // GoToEditor (Left of Center)
  float editorX = centerX - btnW - gap * 0.5f;
  float editorY = bottomY;

  // GoToStageSelect (Right of Center)
  float stageX = centerX + gap * 0.5f;
  float stageY = bottomY;

  if (isClicked) {
    // Check Editor Button
    if (pt.x >= editorX && pt.x <= editorX + btnW && pt.y >= editorY &&
        pt.y <= editorY + btnH) {
      SceneManager::SetTimeScale(1.0f); // Resume time before changing
      SceneManager::ChangeScene("EditorScene");
    }

    // Check StageSelect Button
    if (pt.x >= stageX && pt.x <= stageX + btnW && pt.y >= stageY &&
        pt.y <= stageY + btnH) {
      SceneManager::SetTimeScale(1.0f); // Resume time before changing
      SceneManager::ChangeScene("StageSelectScene");
    }
  }
}

void PauseObject::Draw() {
  float sw = (float)WINDOW_WIDTH;
  float sh = (float)WINDOW_HEIGHT;

  // Semi-transparent overlay
  CDrawUtil::DrawRect(0, 0, sw, sh, 0x88000000);

  // PAUSED Text
  CDrawUtil::DrawText("PAUSED", sw * 0.5f - 30, sh * 0.4f, 0xFFFFFFFF);

  float centerX = sw * 0.5f;
  float bottomY = sh - 100.0f;

  // Button Dimensions
  float btnW = 200.0f;
  float btnH = 50.0f;
  float gap = 20.0f;

  // GoToEditor (Blue)
  float editorX = centerX - btnW - gap * 0.5f;
  float editorY = bottomY;
  CDrawUtil::DrawRect(editorX, editorY, btnW, btnH, 0xFF4169E1);
  CDrawUtil::DrawText("GoToEditor", editorX + 50, editorY + 15, 0xFFFFFFFF);

  // GoToStageSelect (Orange)
  float stageX = centerX + gap * 0.5f;
  float stageY = bottomY;
  CDrawUtil::DrawRect(stageX, stageY, btnW, btnH, 0xFFFFA500);
  CDrawUtil::DrawText("GoToStageSelect", stageX + 40, stageY + 15, 0xFFFFFFFF);
}
