#include "DataCarrier.h"

DataCarrier::DataCarrier() {
  ObjectManager::DontDestroy(this);       // DataCarrierは消されない
  ObjectManager::SetVisible(this, false); // DataCarrierは表示しない

  currentScriptName = "";
  score = 0;
}

DataCarrier::~DataCarrier() { ClearScreenshots(); }

void DataCarrier::ClearScreenshots() {
  for (auto &data : m_screenshotList) {
    if (data.texture)
      data.texture->Release();
    if (data.rawTexture)
      data.rawTexture->Release();
  }
  m_screenshotList.clear();
}

void DataCarrier::Start() {}

void DataCarrier::Update() {}

void DataCarrier::CompleteCurrentStage() {
  if (currentScriptName == "Data/Script/MapField.txt") {
    SetStageCleared(0, true);
  } else if (currentScriptName == "Data/Script/MapDungeon.txt") {
    SetStageCleared(1, true);
  }
}
