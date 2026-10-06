#include "ResultScore.h"
#include "Camera.h"
#include "MyImgui.h"
#include "ObjectManager.h"
#include "SceneManager.h"
#include "ScreenshotManager.h"
#include <cmath> // For pow
#include <cstdio>

namespace {
const float ANIMATION_DURATION_ACCUMULATE = 0.6f; // Buffer加算時間
const float ANIMATION_DURATION_TRANSFER = 0.6f;   // Buffer -> Total 転送時間
const float NEXT_DELAY_TIME = 0.4f;               // 次のスクショまでの待機
} // namespace

ResultScore::ResultScore()
    : m_displayTotalScore(0), timer(0.0f), m_state(State_Init),
      m_currentIndex(-1), m_bufferScore(0), m_currentScreenshotScore(0),
      m_baseTotalScore(0), m_finalTotalScore(0), m_currentRank('E'),
      m_showSaveMessage(false), m_saveMessageTimer(0.0f) {}

ResultScore::~ResultScore() {}

void ResultScore::Update() {
  DataCarrier *dc = ObjectManager::FindGameObject<DataCarrier>();
  if (!dc)
    return;

  switch (m_state) {
  case State_Init:
    m_screenshots = dc->GetScreenshotList();
    m_currentIndex = -1;
    m_displayTotalScore = 0;
    m_bufferScore = 0;
    m_currentRank = 'E';

    if (m_screenshots.empty()) {
      m_finalTotalScore = dc->Score(); // Fallback
      m_displayTotalScore = m_finalTotalScore;
      m_state = State_Finished;
    } else {
      m_state = State_SelectScreenshot;
    }
    break;

  case State_SelectScreenshot:
    m_currentIndex++;
    if (m_currentIndex < (int)m_screenshots.size()) {
      m_currentScreenshotScore = m_screenshots[m_currentIndex].score;
      // バッファに即座に代入 (要件:
      // "指数関数的に増加させる処理を削除しスクショのスコアをそのまま代入")
      m_bufferScore = m_currentScreenshotScore;
      m_baseTotalScore = m_displayTotalScore;

      timer = 0.0f;
      // AccumulateBuffer ステップをスキップして、即座に Transfer (減少) へ
      m_state = State_TransferToTotal;
    } else {
      m_state = State_Finished;
    }
    break;

  case State_AccumulateBuffer:
    // Removed logic, jump to Transfer directly from Select
    m_state = State_TransferToTotal;
    break;

  case State_TransferToTotal:
    timer += SceneManager::DeltaTime();
    {
      float t = timer / ANIMATION_DURATION_TRANSFER;
      if (t >= 1.0f) {
        t = 1.0f;
        m_bufferScore = 0;
        m_displayTotalScore = m_baseTotalScore + m_currentScreenshotScore;
        UpdateRank();
        m_state = State_NextDelay; // 次のスクショへ
        timer = 0.0f;
      } else {
        // 指数関数的に減少させる (Fast -> Slow)
        // t=0 で 1.0, t=1 で 0.0
        // (1-t)^Exp
        float factor = std::pow(1.0f - t, 3.0f);
        m_bufferScore = (int)(m_currentScreenshotScore * factor);

        // トータルスコアは同期して増加
        // Bufferが減った分だけTotalが増える
        int diff = m_currentScreenshotScore - m_bufferScore;
        m_displayTotalScore = m_baseTotalScore + diff;
      }
      UpdateRank();
    }
    break;

  case State_NextDelay:
    timer += SceneManager::DeltaTime();
    if (timer >= NEXT_DELAY_TIME) {
      m_state = State_SelectScreenshot;
    }
    break;

  case State_Finished:
    CheckMouseInput();
    if (m_showSaveMessage) {
      m_saveMessageTimer -= SceneManager::DeltaTime();
      if (m_saveMessageTimer <= 0.0f) {
        m_showSaveMessage = false;
        m_saveMessageTimer = 0.0f;
      }
    }
    break;
  }
}

void ResultScore::UpdateRank() {
  if (m_displayTotalScore >= 8000)
    m_currentRank = 'S';
  else if (m_displayTotalScore >= 5000)
    m_currentRank = 'A';
  else if (m_displayTotalScore >= 3000)
    m_currentRank = 'B';
  else if (m_displayTotalScore >= 1000)
    m_currentRank = 'C';
  else if (m_displayTotalScore >= 500)
    m_currentRank = 'D';
  else
    m_currentRank = 'E';
}

void ResultScore::Draw() {
  // Layout Constants
  ImDrawList *bgList = ImGui::GetBackgroundDrawList();
  ImDrawList *fgList = ImGui::GetForegroundDrawList();

  // Left Panel Box (Draw on Background)
  ImVec2 leftPanelPos((float)LEFT_PANEL_X, (float)LEFT_PANEL_Y);
  ImVec2 leftPanelSize(400, 500);
  bgList->AddRectFilled(leftPanelPos,
                        ImVec2(leftPanelPos.x + leftPanelSize.x,
                               leftPanelPos.y + leftPanelSize.y),
                        IM_COL32(240, 240, 240, 255), 10.0f);
  bgList->AddRect(leftPanelPos,
                  ImVec2(leftPanelPos.x + leftPanelSize.x,
                         leftPanelPos.y + leftPanelSize.y),
                  IM_COL32(50, 100, 255, 255), 10.0f, 0, 5.0f); // Blue Border

  // Left Panel Content
  fgList->AddText(ImVec2(leftPanelPos.x + 20, leftPanelPos.y + 20),
                  IM_COL32(50, 50, 50, 255), "RESULT");

  // RANK (Center Large)
  char rankStr[2] = {m_currentRank, '\0'};
  ImU32 rankColor = IM_COL32(50, 50, 50, 255);
  if (m_currentRank == 'S')
    rankColor = IM_COL32(255, 0, 0, 255);
  else if (m_currentRank == 'A')
    rankColor = IM_COL32(255, 100, 0, 255);
  else if (m_currentRank == 'B')
    rankColor = IM_COL32(0, 0, 255, 255);

  ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Ensure valid font
  fgList->AddText(NULL, 64.0f,
                  ImVec2(leftPanelPos.x + 150, leftPanelPos.y + 120), rankColor,
                  rankStr);
  ImGui::PopFont();

  fgList->AddText(ImVec2(leftPanelPos.x + 240, leftPanelPos.y + 200),
                  IM_COL32(50, 50, 50, 255), "RANK");

  // Separator Line
  float lineY = leftPanelPos.y + 280;
  fgList->AddLine(ImVec2(leftPanelPos.x + 40, lineY),
                  ImVec2(leftPanelPos.x + leftPanelSize.x - 40, lineY),
                  IM_COL32(100, 100, 100, 255), 2.0f);

  // Buffer Score (+ 0)
  char bufStr[64];
  sprintf_s(bufStr, "+ %d", m_bufferScore);
  fgList->AddText(NULL, 32.0f, ImVec2(leftPanelPos.x + 40, lineY + 20),
                  IM_COL32(50, 50, 50, 255), bufStr);

  // Total Score
  char scoreStr[64];
  sprintf_s(scoreStr, "%d", m_displayTotalScore);
  fgList->AddText(NULL, 48.0f, ImVec2(leftPanelPos.x + 60, lineY + 80),
                  IM_COL32(50, 50, 50, 255), scoreStr);
  fgList->AddText(ImVec2(leftPanelPos.x + 300, lineY + 130),
                  IM_COL32(50, 50, 50, 255), "SCORE");

  // Right Panel Box (Draw on BACKGROUND)
  ImVec2 rightPanelPos((float)RIGHT_PANEL_X, (float)RIGHT_PANEL_Y);
  ImVec2 rightPanelSize(450, 600);
  bgList->AddRectFilled(rightPanelPos,
                        ImVec2(rightPanelPos.x + rightPanelSize.x,
                               rightPanelPos.y + rightPanelSize.y),
                        IM_COL32(240, 240, 240, 255), 10.0f);
  bgList->AddRect(rightPanelPos,
                  ImVec2(rightPanelPos.x + rightPanelSize.x,
                         rightPanelPos.y + rightPanelSize.y),
                  IM_COL32(180, 255, 50, 255), 10.0f, 0, 5.0f); // Green Border

  // Right Panel List
  ImGui::SetNextWindowPos(ImVec2(rightPanelPos.x + 20, rightPanelPos.y + 20));
  ImGui::SetNextWindowSize(
      ImVec2(rightPanelSize.x - 40, rightPanelSize.y - 40));

  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
  ImGui::Begin("ScreenshotList", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

  if (m_screenshots.empty()) {
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No Screenshots Taken.");
  }

  int limit = m_currentIndex;
  if (limit < 0)
    limit = -1;
  if (m_state == State_Finished) {
    limit = (int)m_screenshots.size() - 1;
  }

  for (int i = 0; i <= limit; ++i) {
    if (i >= (int)m_screenshots.size())
      break;
    const auto &data = m_screenshots[i];
    ImGui::PushID(i);

    float rowStartY = ImGui::GetCursorPosY();

    // 1. Draw Image
    if (data.texture)
      ImGui::Image((ImTextureID)data.texture, ImVec2(140, 80));
    else
      ImGui::Button("No Image", ImVec2(140, 80));

    // 2. Draw Text
    ImVec4 textColor = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    if (i == m_currentIndex) {
      if (m_state == State_AccumulateBuffer)
        textColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
      else if (m_state == State_TransferToTotal)
        textColor = ImVec4(0.5f, 0.8f, 0.2f, 1.0f);
    } else if (i < m_currentIndex) {
      textColor = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    }

    ImGui::SetCursorPosY(rowStartY + 25);
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMin().x + 150);
    ImGui::TextColored(textColor, "%d Score", data.score);

    // 3. Draw Save Button (if Finished)
    if (m_state == State_Finished) {
      ImGui::SetCursorPosY(rowStartY + 25);
      ImGui::SetCursorPosX(300);
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
      if (ImGui::Button("SAVE")) {
        // Debug: Ensure this block is entered
        m_showSaveMessage = true;
        m_saveMessageTimer = SAVE_MESSAGE_DURATION;

        // Execute Save
        ScreenshotManager::SaveScreenshot(data.rawTexture);
      }
      ImGui::PopStyleColor();
    }

    // 4. Advance Layout Safely
    ImGui::SetCursorPosY(rowStartY);
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMin().x);
    ImGui::Dummy(ImVec2(0.0f, 90.0f));

    ImGui::PopID();
  }
  ImGui::End();
  ImGui::PopStyleColor(); // Pop WindowBg

  // Save Message POPUP
  if (m_showSaveMessage) {
    float msgW = 400.0f;
    float msgH = 50.0f;
    float msgX = (WINDOW_WIDTH - msgW) / 2.0f;
    float msgY = WINDOW_HEIGHT - 100.0f; // 画面下部

    // 背景矩形
    fgList->AddRectFilled(ImVec2(msgX, msgY), ImVec2(msgX + msgW, msgY + msgH),
                          IM_COL32(0, 0, 0, 200), 10.0f);
    // 枠線
    fgList->AddRect(ImVec2(msgX, msgY), ImVec2(msgX + msgW, msgY + msgH),
                    IM_COL32(255, 255, 255, 255), 10.0f);

    // テキスト
    std::string msg = MyImgui::SJIStoUTF8("スクリーンショットを保存しました");
    const char *text = msg.c_str();
    ImVec2 textSize = ImGui::CalcTextSize(text);
    fgList->AddText(ImVec2(msgX + (msgW - textSize.x) / 2.0f,
                           msgY + (msgH - textSize.y) / 2.0f),
                    IM_COL32(255, 255, 255, 255), text);
  }
}
void ResultScore::CheckMouseInput() {
  // ImGui handles mouse input.
}
