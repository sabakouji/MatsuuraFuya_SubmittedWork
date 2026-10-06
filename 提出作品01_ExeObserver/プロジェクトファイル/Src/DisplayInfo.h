#pragma once
#include "Object3D.h"

/// <summary>
/// 画面に各種情報を表示する処理
/// </summary>
class DisplayInfo : public Object3D {
public:
  DisplayInfo();
  ~DisplayInfo();

  void Update() override;
  void Draw() override;

private:
  CSpriteImage *image;
  CSpriteImage *m_hpFrame;
  CSpriteImage *m_hpFill;
  CSpriteImage *m_targetFrame;
  CSprite *sprite;

  // Pause UI Members
  bool m_isPaused = false;
  enum class ConfirmMode { None, StageSelect, Editor };
  ConfirmMode m_confirmMode = ConfirmMode::None;

  CSpriteImage *m_exitTab;
  CSpriteImage *m_gotoEditor;
  CSpriteImage *m_caveatTab;       // For Editor
  CSpriteImage *m_caveatTabSelect; // For Stage Select
  CSpriteImage *m_btnYes;
  CSpriteImage *m_btnNo;

public:
  void SetPaused(bool paused);
  bool IsPaused() const { return m_isPaused; }

private:
  CSpriteImage *m_gameOverImage;
  float m_gameOverTimer;
  float m_retryTimer;
  bool m_isGameOverSequence;
};