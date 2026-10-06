#pragma once
#include "SceneBase.h"
#include "Sprite3D.h"

class ModeSelectScene : public SceneBase {
public:
  ModeSelectScene();
  ~ModeSelectScene();
  void Update() override;
  void Draw() override;

private:
  bool m_confirmExit;

  CSpriteImage* m_bgImage;     // SelectBG.png
  CSpriteImage *m_imgStage;    // Stage.png
  CSpriteImage *m_imgEditor;   // AIEditor.png
  CSpriteImage *m_imgExit;     // Exit_Button.png
  CSpriteImage *m_imgBar;      // FrontBar.png

  CSpriteImage* m_tabImage;    // ExitTab.png
  CSpriteImage* m_btnYesImage; // ExitButton_Exit.png
  CSpriteImage* m_btnNoImage;  // ExitButton_Cancel.png
  CSprite *m_sprite;
};
