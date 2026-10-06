#include "CDrawUtil.h"
#include <algorithm>
#include <cmath>

//-----------------------------------------------------------------------------
// 描画ユーティリティ
namespace CDrawUtil {
static inline ImU32 ARGBtoIMCOL32(int argb) {
  const ImU32 a = (argb >> 24) & 0xFF;
  const ImU32 r = (argb >> 16) & 0xFF;
  const ImU32 g = (argb >> 8) & 0xFF;
  const ImU32 b = (argb) & 0xFF;
  return IM_COL32(r, g, b, a);
}

static ImDrawList* g_pOverrideDrawList = nullptr;

void SetDrawList(ImDrawList* dl) {
    g_pOverrideDrawList = dl;
}

ImDrawList* GetDrawList() {
    if (g_pOverrideDrawList) return g_pOverrideDrawList;
    return ImGui::GetBackgroundDrawList();
}

static inline ImDrawList *DL() { return GetDrawList(); }
//-----------------------------------------------------------------------------

void DrawRect(float x, float y, float w, float h, int color) {
  const ImU32 col = ARGBtoIMCOL32(color);
  const ImVec2 pmin(x, y);
  const ImVec2 pmax(x + w, y + h);
  const float rounding = 6.0f; // 角丸半径
  const ImDrawFlags flags = ImDrawFlags_RoundCornersAll;
  DL()->AddRectFilled(pmin, pmax, col, rounding, flags);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ヘッダー用の矩形を描画
void DrawRect_Header(float x, float y, float w, float h, int color) {
  const ImU32 col = ARGBtoIMCOL32(color);
  const ImVec2 pmin(x, y);
  const ImVec2 pmax(x + w, y + h);
  const float rounding = 6.0f; // 角丸半径
  const ImDrawFlags flags = ImDrawFlags_RoundCornersBottom;
  DL()->AddRectFilled(pmin, pmax, col, rounding, flags);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 円を描画
void DrawCircle(float x, float y, float r, int color) {
  const ImU32 col = ARGBtoIMCOL32(color);
  DL()->AddCircleFilled(ImVec2(x, y), r, col, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// テキストを描画
void DrawText(const char *text, float x, float y, int color) {
  if (!text)
    return;
  const ImU32 col = ARGBtoIMCOL32(color);
  DL()->AddText(ImVec2(x, y), col, text);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ベジエ曲線を描画
void DrawBezierCurve(const VECTOR2 &p1, const VECTOR2 &p2, int color) {
  const ImU32 col = ARGBtoIMCOL32(color);
  const ImVec2 p0v(p1.x, p1.y);
  const ImVec2 p3v(p2.x, p2.y);

  // ハンドルの長さを計算
  const float dx = p3v.x - p0v.x;
  const float dy = p3v.y - p0v.y;
  const float dist = std::sqrt(dx * dx + dy * dy);
  const float handle = std::max<float>(40.0f, dist * 0.30f);

  // ベジエ曲線の制御点
  const ImVec2 p1v(p0v.x + handle, p0v.y);
  const ImVec2 p2v(p3v.x - handle, p3v.y);

  const float thickness = 2.0f;
  DL()->AddBezierCubic(p0v, p1v, p2v, p3v, col, thickness);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ベジエ曲線を描画（入力/出力ピンの方向に応じてハンドルの位置を調整）
void DrawBezierCurve(const VECTOR2 &p1, const VECTOR2 &p2, bool startIsInput,
                     bool endIsInput, int color) {
  const ImU32 col = ARGBtoIMCOL32(color);
  const ImVec2 p0v(p1.x, p1.y);
  const ImVec2 p3v(p2.x, p2.y);

  const float dx = p3v.x - p0v.x;
  const float dy = p3v.y - p0v.y;
  const float dist = std::sqrt(dx * dx + dy * dy);
  float handle = std::max<float>(40.0f, dist * 0.30f);
  float maxHandle = std::fabs(dx) * 0.5f;

  if (handle > maxHandle)
    handle = maxHandle;

  ImVec2 p1v = startIsInput ? ImVec2(p0v.x - handle, p0v.y)
                            : ImVec2(p0v.x + handle, p0v.y);

  ImVec2 p2v = endIsInput ? ImVec2(p3v.x - handle, p3v.y)
                          : ImVec2(p3v.x + handle, p3v.y);

  const float thickness = 2.0f;
  DL()->AddBezierCubic(p0v, p1v, p2v, p3v, col, thickness);
}
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------