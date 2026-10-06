#include "GUIHelper.h"
#include "CDrawUtil.h" // Fallback or additional drawing
#include "MyImgui.h"   // Assuming we use Custom ImGui wrapper or similar
#include <iostream>

void GUIHelper::Initialize() {
  // Init resources if needed
}

bool GUIHelper::DrawButton(const std::string &label, const VECTOR2 &pos,
                           const VECTOR2 &size) {
  // Placeholder for actual button logic.
  // In a real ImGui scenario, we would use ImGui::Button.
  // Since we don't have full ImGui access confirmed in this limited view,
  // we simulate or use what we saw in "MyImgui" if possible.

  // For now, let's assume we can print to log or just return false (no
  // interaction yet) Real implementation would link to Input system.

  // Using MyImgui static methods if available (guessing API based on name)
  // MyImgui::Begin(), ...

  return false;
}

void GUIHelper::DrawBar(const std::string &label, float value, float maxValue,
                        const VECTOR2 &pos, const VECTOR2 &size,
                        const VECTOR4 &color) {
  ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(size.x, size.y), ImGuiCond_Always);
  std::string wName = "Bar_" + label;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  if (ImGui::Begin(wName.c_str(), nullptr,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoInputs |
                       ImGuiWindowFlags_NoBackground)) {
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                          ImVec4(color.x, color.y, color.z, color.w));
    ImGui::ProgressBar((maxValue > 0 ? value / maxValue : 0),
                       ImVec2(size.x, size.y), "");
    ImGui::PopStyleColor();
    ImGui::End();
  }
  ImGui::PopStyleVar();
}

void GUIHelper::DrawLabel(const std::string &text, const VECTOR2 &pos,
                          const VECTOR4 &color) {
  ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Always);
  std::string wName = "Lbl_" + text;
  if (ImGui::Begin(wName.c_str(), nullptr,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoInputs |
                       ImGuiWindowFlags_NoBackground |
                       ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextColored(ImVec4(color.x, color.y, color.z, color.w), "%s",
                       text.c_str());
    ImGui::End();
  }
}

void GUIHelper::DrawLabelWithBackground(const std::string &text,
                                        const VECTOR2 &pos,
                                        const VECTOR4 &textColor,
                                        const VECTOR4 &bgColor) {
  ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(bgColor.w); // Use alpha from color
  std::string wName = "LblBg_" + text;

  // Use a unique ID to avoid conflicts if text is same?
  // wName should be unique-ish.

  ImGui::PushStyleColor(ImGuiCol_WindowBg,
                        ImVec4(bgColor.x, bgColor.y, bgColor.z, bgColor.w));

  if (ImGui::Begin(wName.c_str(), nullptr,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoInputs |
                       ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextColored(
        ImVec4(textColor.x, textColor.y, textColor.z, textColor.w), "%s",
        text.c_str());
    ImGui::End();
  }
  ImGui::PopStyleColor();
}

void GUIHelper::DrawScorePanel(int score, int rank, const VECTOR2 &pos) {
  ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.7f); // Transparent background
  if (ImGui::Begin(
          "Result Overlay", nullptr,
          ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
              ImGuiWindowFlags_NoSavedSettings |
              ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "RESULT");
    ImGui::Separator();
    ImGui::Text("SCORE: %d", score);
    std::string rankStr(1, (char)rank);
    ImGui::Text("RANK : %s", rankStr.c_str());
    ImGui::End();
  }
}
