#include "SceneFactory.h"
#include "ClearScene.h"
#include "DataCarrier.h"
#include "EditorScene.h"
#include "ModeSelectScene.h"

#include "OverScene.h"
#include "PlayScene.h"
#include "StageSelectScene.h"
#include "TitleScene.h"
#include <assert.h>
#include <windows.h>

std::unique_ptr<SceneBase> SceneFactory::CreateFirst() {
  SingleInstantiate<DataCarrier>();
  return std::make_unique<TitleScene>();
}

std::unique_ptr<SceneBase> SceneFactory::Create(const std::string &name) {
  if (name == "TitleScene") {
    return std::make_unique<TitleScene>();
  }
  if (name.substr(0, 9) == "PlayScene") {
    return std::make_unique<PlayScene>();
  }
  if (name == "ClearScene") {
    return std::make_unique<ClearScene>();
  }
  if (name == "OverScene") {
    return std::make_unique<OverScene>();
  }
  if (name == "EditorScene") {
    return std::make_unique<EditorScene>();
  }
  if (name == "ModeSelectScene") {
    return std::make_unique<ModeSelectScene>();
  }
  if (name == "StageSelectScene") {
    return std::make_unique<StageSelectScene>();
  }
  assert(false);
  return nullptr;
}