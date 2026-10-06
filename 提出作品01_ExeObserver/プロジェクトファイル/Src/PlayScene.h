#pragma once
#include "SceneBase.h"

class PlayScene : public SceneBase {
public:
  PlayScene();
  ~PlayScene();
  void Update() override;
  void Draw() override;

private:
  class PauseObject *m_pauseObj;
  bool m_isPaused;

  class Object3D *m_btnObj;
  bool m_isStartWait;
};
