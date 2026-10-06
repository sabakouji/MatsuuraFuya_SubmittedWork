// 22行目 [DisplayInfo] 初期化
// 60行目 [~DisplayInfo] 破棄
// 75行目 [SetPaused] ポーズ設定
// 84行目 [Draw] 描画処理

#include "DisplayInfo.h"
#include "AudioManager.h" // Added for Sound
#include "CDrawUtil.h"    // Added for CDrawUtil::DrawText
#include "Camera.h"
#include "DataCarrier.h"
#include "Executor.h"
#include "GUIHelper.h" // Include GUIHelper
#include "ObjectManager.h"
#include "Player.h"

namespace {
const int ScoreMax = 1000; // スコアバーの表示上のスコアの最大値
}

#include "FadeObject.h"
#include "SceneManager.h"

DisplayInfo::DisplayInfo() {
  m_hpFrame = new CSpriteImage(GameDevice()->m_pShader);
  m_hpFrame->Load("Data/Image/HP_Frame.png");

  m_hpFill = new CSpriteImage(GameDevice()->m_pShader);
  m_hpFill->Load("Data/Image/HP_Bar.png");

  m_targetFrame = new CSpriteImage(GameDevice()->m_pShader);
  m_targetFrame->Load("Data/Image/TargetFrame.png");

  // Pause UI Imports
  m_exitTab = new CSpriteImage(GameDevice()->m_pShader);
  m_exitTab->Load("Data/Image/GotoSelectScene.png");

  m_gotoEditor = new CSpriteImage(GameDevice()->m_pShader);
  m_gotoEditor->Load("Data/Image/GotoEditor.png");

  m_caveatTab = new CSpriteImage(GameDevice()->m_pShader);
  m_caveatTab->Load("Data/Image/CaveatTab.png");

  m_caveatTabSelect = new CSpriteImage(GameDevice()->m_pShader);
  m_caveatTabSelect->Load("Data/Image/CaveatTab_SelectScene.png");

  m_btnYes = new CSpriteImage(GameDevice()->m_pShader);
  m_btnYes->Load("Data/Image/Exec_Button.png");

  m_btnNo = new CSpriteImage(GameDevice()->m_pShader);
  m_btnNo->Load("Data/Image/ExitButton_Cancel.png");

  sprite = new CSprite();

  m_gameOverImage = new CSpriteImage(GameDevice()->m_pShader);
  m_gameOverImage->Load("Data/Image/GameOver.png");
  m_gameOverTimer = 0.0f;
  m_retryTimer = 0.0f;
  m_isGameOverSequence = false;

  SetPriority(-9999);
  SetDrawOrder(-10000);
}

DisplayInfo::~DisplayInfo() {
  SAFE_DELETE(m_hpFrame);
  SAFE_DELETE(m_hpFill);
  SAFE_DELETE(m_targetFrame);
  SAFE_DELETE(sprite);

  SAFE_DELETE(m_exitTab);
  SAFE_DELETE(m_gotoEditor);
  SAFE_DELETE(m_caveatTab);
  SAFE_DELETE(m_caveatTabSelect);
  SAFE_DELETE(m_btnYes);
  SAFE_DELETE(m_btnNo);
  SAFE_DELETE(m_gameOverImage);
}

void DisplayInfo::SetPaused(bool paused) {
  m_isPaused = paused;
  if (!m_isPaused) {
    m_confirmMode = ConfirmMode::None;
  }
}

void DisplayInfo::Update() {}

void DisplayInfo::Draw() {
  // Check if screenshot is requested
  std::list<Camera *> cameras = ObjectManager::FindGameObjects<Camera>();
  if (!cameras.empty()) {
    Camera *cam = cameras.front();
    if (cam && cam->IsScreenshotRequested()) {
      cam->TakeScreenshot();
    }
  }

  // Find Executor
  std::list<Executor *> executors = ObjectManager::FindGameObjects<Executor>();
  if (executors.empty())
    return;

  Executor *obj = executors.front();
  if (!obj)
    return;

  float sw = (float)WINDOW_WIDTH;
  float sh = (float)WINDOW_HEIGHT;

  // Draw HP Bar (Bottom Left) -- Always Visible
  if (m_hpFrame && m_hpFill && sprite) {
    float frameW = (float)m_hpFrame->m_dwImageWidth;
    float frameH = (float)m_hpFrame->m_dwImageHeight;
    float barX = 20;
    float barY = sh - frameH - 20;

    // Draw Frame
    sprite->Draw(m_hpFrame, barX, barY, 0, 0, frameW, frameH);

    // Draw Fill
    // Use native size for fill bar
    float maxFillW = (float)m_hpFill->m_dwImageWidth;
    float fillH = (float)m_hpFill->m_dwImageHeight;

    float fillOffsetX = 120.0f;
    float fillOffsetY = (frameH - fillH) / 2.0f;

    float hpRatio = obj->HpRatio();
    if (hpRatio < 0)
      hpRatio = 0;
    if (hpRatio > 1)
      hpRatio = 1;

    float currentFillW = maxFillW * hpRatio;

    if (currentFillW > 0)
      sprite->Draw(m_hpFill, barX + fillOffsetX, barY + fillOffsetY, 0, 0,
                   currentFillW, fillH);
  }

  // Game Over Logic
  if (obj->HpRatio() <= 0.0f) {
    if (!m_isGameOverSequence) {
      m_isGameOverSequence = true;
      m_gameOverTimer = 0.0f;
      m_retryTimer = 0.0f;

      // Play Sound
      AudioManager::Audio("GameOver")->Play(0);
    }

    float duration = 0.5f;

    if (m_gameOverTimer < duration) {
      m_gameOverTimer += SceneManager::UnscaledDeltaTime();
    } else if (m_gameOverTimer > duration) {
      m_gameOverTimer = duration;
    }

    if (m_gameOverImage) {
      float imgW = (float)m_gameOverImage->m_dwImageWidth;
      float imgH = (float)m_gameOverImage->m_dwImageHeight;

      float startX = -imgW; // Left Edge
      float endX = (sw - imgW) / 2.0f;

      float t = m_gameOverTimer / duration; // Normalize 0.0 to 1.0
      float currentX = startX + (endX - startX) * t;

      float drawY = (sh - imgH) / 2.0f;

      sprite->Draw(m_gameOverImage, currentX, drawY, 0, 0, imgW, imgH);
    }

    if (m_gameOverTimer >= duration) {
      m_retryTimer += SceneManager::UnscaledDeltaTime();

      if (m_retryTimer >= 1.0f) {
        SceneManager::Reload();
      }
    }

    return;
  }

  // If Paused, Draw Pause UI and Handle Input, then Return
  if (m_isPaused) {
    if (!sprite)
      return;

    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GameDevice()->m_pMain->m_hWnd, &pt);
    bool isClicked = GameDevice()->m_pDI->CheckMouse(KD_TRG, 0);

    if (m_confirmMode == ConfirmMode::None) {
      // Normal Pause View

      // Left Top: Exit Tab
      if (m_exitTab) {
        float w = (float)m_exitTab->m_dwImageWidth;
        float h = (float)m_exitTab->m_dwImageHeight;
        float x = 0;
        float y = 0;
        sprite->Draw(m_exitTab, x, y, 0, 0, w, h);

        if (isClicked && pt.x >= x && pt.x <= x + w && pt.y >= y &&
            pt.y <= y + h) {
          m_confirmMode = ConfirmMode::StageSelect;
        }
      }

      // Right Bottom: AI Editor (Above HP bar or just bottom right?)
      if (m_gotoEditor) {
        float w = (float)m_gotoEditor->m_dwImageWidth;
        float h = (float)m_gotoEditor->m_dwImageHeight;
        float x = sw - w;
        float y = sh - h - 50.0f; // Little bit offset from bottom

        sprite->Draw(m_gotoEditor, x, y, 0, 0, w, h);

        if (isClicked && pt.x >= x && pt.x <= x + w && pt.y >= y &&
            pt.y <= y + h) {
          m_confirmMode = ConfirmMode::Editor;
        }
      }

    } else {
      // Confirmation Dialog
      CSpriteImage *pTab = (m_confirmMode == ConfirmMode::StageSelect)
                               ? m_caveatTabSelect
                               : m_caveatTab;

      if (pTab) {
        float boxW = (float)pTab->m_dwImageWidth;
        float boxH = (float)pTab->m_dwImageHeight;
        float boxX = sw * 0.5f - boxW * 0.5f;
        float boxY = sh * 0.5f - boxH * 0.5f;

        sprite->Draw(pTab, boxX, boxY, 0, 0, boxW, boxH);

        // Buttons
        // "Image 1768328372908.png" shows Red "Run" button left, White "Cancel"
        // button right.

        float btnHeight = 60.0f; // Approx from image/logic
        if (m_btnYes)
          btnHeight = (float)m_btnYes->m_dwImageHeight;

        float halfW = boxW / 2.0f;
        float btnY = boxY + boxH - btnHeight;

        // Yes Button (Left Half)
        if (m_btnYes) {
          float btnW = (float)m_btnYes->m_dwImageWidth;
          float btnH = (float)m_btnYes->m_dwImageHeight;

          // Calculate draw position
          float drawX = boxX + 2;
          float drawY = boxH + boxY; // Positioned below/at bottom of box

          // Use drawX for drawing
          sprite->Draw(m_btnYes, drawX, drawY, 0, 0, btnW, btnH);

          // Update hit detection to match draw position
          if (isClicked && pt.x >= drawX && pt.x <= drawX + btnW &&
              pt.y >= drawY && pt.y <= drawY + btnH) {
            // Execute Transition

            // Capture current mode before resetting
            ConfirmMode targetMode = m_confirmMode;

            SetPaused(false);
            // CRITICAL: Unpause time so FadeObject can update!
            SceneManager::SetTimeScale(1.0f);

            FadeObject *fade = ObjectManager::CreateGameObject<FadeObject>();
            fade->SetShowLoading(true);
            ObjectManager::DontDestroy(fade);

            // Capture targetMode by VALUE
            fade->StartFadeOut(1.0f, [targetMode]() {
              if (targetMode == ConfirmMode::StageSelect) {
                SceneManager::ChangeScene("StageSelectScene");
              } else {
                SceneManager::ChangeScene("EditorScene");
              }
            });
          }
        }

        // No Button (Right Half)
        if (m_btnNo) {
          float btnW = (float)m_btnNo->m_dwImageWidth;
          float btnH = (float)m_btnNo->m_dwImageHeight;

          float btnX = boxX + halfW;
          // Calculate draw position
          float drawX = boxX + btnW + 2.0f;
          float drawY = boxH + boxY;

          sprite->Draw(m_btnNo, drawX, drawY, 0, 0, btnW, btnH);

          // Update hit detection to match draw position
          if (isClicked && pt.x >= drawX && pt.x <= drawX + btnW &&
              pt.y >= drawY && pt.y <= drawY + btnH) {
            m_confirmMode = ConfirmMode::None;
          }
        }
      }
    }

    return; // End Draw for Pause Mode (Skip TargetFrame etc)
  }

  // 1. Draw Target Frame (Center)
  if (m_targetFrame && sprite) {
    float frameW = 437;
    float frameH = 301;
    float frameX = sw * 0.5f - frameW * 0.5f;
    float frameY = sh * 0.5f - frameH * 0.5f;
    sprite->Draw(m_targetFrame, frameX, frameY, 0, 0, frameW, frameH);
  }

  // 3. Draw Executor Info Text (Running Node, etc.)
  std::string nodeName = obj->GetCurrentRunningNode();
  std::string text = "Running: " + (nodeName.empty() ? "-" : nodeName);

  // Draw with semi-transparent black background
  // Text: White, Bg: Black (0.5 alpha)
  GUIHelper::DrawLabelWithBackground(
      text, VECTOR2(20, 110), VECTOR4(1, 1, 1, 1), VECTOR4(0, 0, 0, 0.5f));

  // Draw Result Screen if Camera has screenshot
  std::list<Camera *> incameras = ObjectManager::FindGameObjects<Camera>();
  if (!incameras.empty()) {
    Camera *cam = incameras.front();
    if (cam && cam->HasScreenshot()) {
      // Draw Center Screen
      GUIHelper::DrawScorePanel(cam->GetScore(), cam->GetRank(),
                                VECTOR2(400, 200));
    }
  }
}
