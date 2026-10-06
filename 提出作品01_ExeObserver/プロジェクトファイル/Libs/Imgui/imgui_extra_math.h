//------------------------------------------------------------------------------
// VERSION 0.9.1
// LICENSE: public domain (Michal Cichon)
// Stub: missing imgui-node-editor dependency
//------------------------------------------------------------------------------
# ifndef __IMGUI_EXTRA_MATH_H__
# define __IMGUI_EXTRA_MATH_H__
# pragma once

# include <imgui.h>
# include <imgui_internal.h>

//------------------------------------------------------------------------------
// ImLine
struct ImLine
{
    ImVec2 A;
    ImVec2 B;
};

//------------------------------------------------------------------------------
// ImLength / ImNormalized  (ImLengthSqr defined in imgui_internal.h)
static inline float ImLength(const ImVec2& v)
{
    return ImSqrt(ImLengthSqr(v));
}

static inline ImVec2 ImNormalized(const ImVec2& v)
{
    float len = ImLength(v);
    if (len < 1e-6f) return ImVec2(0.0f, 0.0f);
    return v * (1.0f / len);
}

//------------------------------------------------------------------------------
// ImRect_ClosestPoint  (3-arg + 4-arg overloads)
static inline bool ImRect_IsEmpty(const ImRect& r)
{
    return r.Min.x >= r.Max.x || r.Min.y >= r.Max.y;
}

static inline ImVec2 ImRect_ClosestPoint(const ImRect& r, const ImVec2& p, bool onEdge)
{
    if (!onEdge && r.Contains(p))
        return p;
    return ImVec2(ImClamp(p.x, r.Min.x, r.Max.x),
                  ImClamp(p.y, r.Min.y, r.Max.y));
}

static inline ImVec2 ImRect_ClosestPoint(const ImRect& r, const ImVec2& p, bool onEdge, float extraThickness)
{
    if (extraThickness > 0.0f)
    {
        ImRect er(r.Min.x - extraThickness, r.Min.y - extraThickness,
                  r.Max.x + extraThickness, r.Max.y + extraThickness);
        return ImRect_ClosestPoint(er, p, onEdge);
    }
    return ImRect_ClosestPoint(r, p, onEdge);
}

//------------------------------------------------------------------------------
// ImRect_ClosestLine
static inline ImLine ImRect_ClosestLine(const ImRect& a, const ImRect& b)
{
    ImVec2 cb((b.Min.x + b.Max.x) * 0.5f, (b.Min.y + b.Max.y) * 0.5f);
    ImLine result;
    result.A = ImRect_ClosestPoint(a, cb, true);
    result.B = ImRect_ClosestPoint(b, result.A, true);
    result.A = ImRect_ClosestPoint(a, result.B, true);
    return result;
}

static inline ImLine ImRect_ClosestLine(const ImRect& a, const ImRect& b, float extentA, float extentB)
{
    ImRect ea(a.Min.x - extentA, a.Min.y - extentA, a.Max.x + extentA, a.Max.y + extentA);
    ImRect eb(b.Min.x - extentB, b.Min.y - extentB, b.Max.x + extentB, b.Max.y + extentB);
    return ImRect_ClosestLine(ea, eb);
}

//------------------------------------------------------------------------------
// ImEasing
namespace ImEasing {
    template<typename T>
    inline T EaseOutQuad(T b, T c, float t)
    {
        return b + c * (2.0f * t - t * t);
    }
}

# endif // __IMGUI_EXTRA_MATH_H__
