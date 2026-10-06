#pragma once
#include "Object3D.h"
#include <d3d11.h>
#include <vector>

/// <summary>
/// ゲームの進行に伴う様々なデータを保存するクラス
/// １．スクリプト名
/// ２．スコア値
/// </summary>
class DataCarrier : public Object3D {
public:
  DataCarrier();
  ~DataCarrier();

  void Start() override;
  void Update() override;

  // スクリーンショットデータ構造体
  struct ScreenshotData {
    int score;
    char rank;
    struct ID3D11ShaderResourceView *texture;
    struct ID3D11Texture2D *rawTexture; // 保存用に生テクスチャも保持
    int width;
    int height;
  };

  void AddScore(int inScore) { score += inScore; }
  void ClearScore() { score = 0; }
  int Score() { return score; }
  void SetScriptName(std::string name) { currentScriptName = name; }
  std::string ScriptName() { return currentScriptName; }

  void AddScreenshot(const ScreenshotData &data) {
    m_screenshotList.push_back(data);
  }
  const std::vector<ScreenshotData> &GetScreenshotList() const {
    return m_screenshotList;
  }
  void ClearScreenshots();

  // Stage Clear Management
  void SetStageCleared(int index, bool cleared) {
    if (index >= 0) {
      if (index >= (int)m_clearedStages.size()) {
        m_clearedStages.resize(index + 1, false);
      }
      m_clearedStages[index] = cleared;
    }
  }

  void
  CompleteCurrentStage(); // Method to mark current script's stage as cleared

  bool IsStageCleared(int index) {
    if (index >= 0 && index < (int)m_clearedStages.size()) {
      return m_clearedStages[index];
    }
    return false;
  }

private:
  std::string currentScriptName;
  int score;
  std::vector<ScreenshotData> m_screenshotList;
  std::vector<bool> m_clearedStages;
};