#pragma once
#include "GameObject.h"

struct ImDrawList;

namespace CDrawUtil {
    // 描画関数の宣言
    void DrawRect(float x, float y, float w, float h, int color);
    void DrawRect_Header(float x, float y, float w, float h, int color);
    void DrawCircle(float x, float y, float r, int color);
    void DrawText(const char* text, float x, float y, int color);
    void DrawBezierCurve(const VECTOR2& p1, const VECTOR2& p2, int color);
    void DrawBezierCurve(const VECTOR2& p1, const VECTOR2& p2, bool startIsInput, bool endIsInput, int color);

    void SetDrawList(ImDrawList* dl);
    ImDrawList* GetDrawList();

    // 色定義（constexpr なのでヘッダーでOK）
    constexpr int COLOR_NODE_BACKGROUND = 0xFF323232;
    constexpr int COLOR_NODE_HEADER_DEFAULT = 0xFF246bb7;
    constexpr int COLOR_NODE_HEADER_SELECTED = 0xFF00BFFF;
    constexpr int COLOR_PIN_CONTROL = 0xFF00FF00;
    constexpr int COLOR_PIN_DATA = 0xFFFFFF00;
    constexpr int COLOR_LINK = 0xFF808080;
    constexpr int COLOR_TEXT = 0xFFFFFFFF;
    constexpr int COLOR_BUTTON_NORMAL = 0xFF00BFFF;
    constexpr int COLOR_BUTTON_PRESSED = 0xFF174a80;
    constexpr int COLOR_BUTTON_HOVER = 0xFF00FF00;
}