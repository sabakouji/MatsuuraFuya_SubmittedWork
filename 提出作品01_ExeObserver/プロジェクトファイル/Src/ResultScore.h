#pragma once
#include "DataCarrier.h"
#include "GameObject.h"
#include <string>
#include <vector>

class ResultScore : public GameObject {
public:
  ResultScore();
  ~ResultScore();
  void Update() override;
  void Draw() override;

private:
  int m_displayTotalScore; // トータルスコア (表示用)
  float timer;

  enum State {
    State_Init,
    State_SelectScreenshot, // 次のスクショを選択
    State_AccumulateBuffer, // バッファスコアへの加算 (0 -> Score)
    State_TransferToTotal,  // トータルスコアへの転送 (Buffer -> 0, Total ->
                            // Total+Score)
    State_NextDelay,        // 待機
    State_Finished
  };
  State m_state;

  int m_currentIndex;
  std::vector<DataCarrier::ScreenshotData> m_screenshots;

  int m_bufferScore;            // 左側に表示される "+ N" の数値
  int m_currentScreenshotScore; // 現在対象のスクショのスコア
  int m_baseTotalScore;         // アニメーション開始前のトータルスコア
  int m_finalTotalScore;        // 最終的なトータルスコア
                                // (DataCarrierの合計と一致するはず)

  char m_currentRank;

  // UI Layout
  static const int LEFT_PANEL_X = 100;
  static const int LEFT_PANEL_Y = 100;
  static const int RIGHT_PANEL_X = 650;
  static const int RIGHT_PANEL_Y = 100;

  // Message Display
  bool m_showSaveMessage;
  float m_saveMessageTimer;
  const float SAVE_MESSAGE_DURATION = 3.0f;

  void UpdateRank();
  void CheckMouseInput();
  void DrawScreenshot(int x, int y, int w, int h,
                      const DataCarrier::ScreenshotData &data);
};
