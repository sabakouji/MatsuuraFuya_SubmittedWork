#pragma once
#include "SceneBase.h"

class FadeObject;

class TitleScene : public SceneBase {
public:
  TitleScene();
  ~TitleScene();
  void Update() override;
  void Draw() override;

private:
  FadeObject *m_fade;
  bool m_isFading;
};
