#pragma once
#include "MyMath.h"
#include <string>

class GUIHelper {
public:
  static void Initialize();

  // Draw a simple button
  static bool DrawButton(const std::string &label, const VECTOR2 &pos,
                         const VECTOR2 &size);

  // Draw a progress bar (e.g. HP)
  static void DrawBar(const std::string &label, float value, float maxValue,
                      const VECTOR2 &pos, const VECTOR2 &size,
                      const VECTOR4 &color);

  // Draw text with background
  static void DrawLabel(const std::string &text, const VECTOR2 &pos,
                        const VECTOR4 &color);

  // Draw text with background box
  static void DrawLabelWithBackground(const std::string &text,
                                      const VECTOR2 &pos,
                                      const VECTOR4 &textColor,
                                      const VECTOR4 &bgColor);

  // Draw Result Score Panel
  static void DrawScorePanel(int score, int rank, const VECTOR2 &pos);
};
