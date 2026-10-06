//------------------------------------------------------------------------------
// VERSION 0.1
// LICENSE: public domain (Michal Cichon)
// Stub: missing imgui-node-editor dependency
//------------------------------------------------------------------------------
# ifndef __IMGUI_BEZIER_MATH_H__
# define __IMGUI_BEZIER_MATH_H__
# pragma once

# include <imgui.h>
# include <imgui_internal.h>
# include "imgui_extra_math.h"
# include <cfloat>

//------------------------------------------------------------------------------
// Data types

struct ImCubicBezierPoints
{
    ImVec2 P0, P1, P2, P3;
};

struct ImCubicBezierSubdivideSample
{
    ImVec2 Point;
    ImVec2 Tangent;
};

struct ImCubicBezierFixedStepSample
{
    float  Length;
    ImVec2 Point;
};

struct ImCubicBezierIntersectResult
{
    int    Count;
    ImVec2 Points[3];
};

struct ImProjectResult
{
    ImVec2 Point;
    float  Time;
    float  Distance;
};

//------------------------------------------------------------------------------
// Sample point on cubic bezier at t=[0,1]
static inline ImVec2 ImCubicBezierSample(
    const ImVec2& p0, const ImVec2& p1,
    const ImVec2& p2, const ImVec2& p3, float t)
{
    float u  = 1.0f - t;
    float b0 = u * u * u;
    float b1 = 3.0f * u * u * t;
    float b2 = 3.0f * u * t * t;
    float b3 = t * t * t;
    return ImVec2(b0*p0.x + b1*p1.x + b2*p2.x + b3*p3.x,
                  b0*p0.y + b1*p1.y + b2*p2.y + b3*p3.y);
}

//------------------------------------------------------------------------------
// Tangent vector at t
static inline ImVec2 ImCubicBezierTangent(
    const ImVec2& p0, const ImVec2& p1,
    const ImVec2& p2, const ImVec2& p3, float t)
{
    float u  = 1.0f - t;
    float b0 = 3.0f * u * u;
    float b1 = 6.0f * u * t;
    float b2 = 3.0f * t * t;
    return ImVec2(b0*(p1.x-p0.x) + b1*(p2.x-p1.x) + b2*(p3.x-p2.x),
                  b0*(p1.y-p0.y) + b1*(p2.y-p1.y) + b2*(p3.y-p2.y));
}

//------------------------------------------------------------------------------
// Arc length by numerical integration
static inline float ImCubicBezierLength(
    const ImVec2& p0, const ImVec2& p1,
    const ImVec2& p2, const ImVec2& p3)
{
    const int N = 64;
    float len = 0.0f;
    ImVec2 prev = p0;
    for (int i = 1; i <= N; ++i)
    {
        ImVec2 cur = ImCubicBezierSample(p0, p1, p2, p3, (float)i / N);
        len += ImLength(cur - prev);
        prev = cur;
    }
    return len;
}

static inline float ImCubicBezierLength(const ImCubicBezierPoints& c)
{
    return ImCubicBezierLength(c.P0, c.P1, c.P2, c.P3);
}

//------------------------------------------------------------------------------
// Bounding rect by sampling
static inline ImRect ImCubicBezierBoundingRect(
    const ImVec2& p0, const ImVec2& p1,
    const ImVec2& p2, const ImVec2& p3)
{
    ImRect r(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
    const int N = 32;
    for (int i = 0; i <= N; ++i)
    {
        ImVec2 pt = ImCubicBezierSample(p0, p1, p2, p3, (float)i / N);
        if (pt.x < r.Min.x) r.Min.x = pt.x;
        if (pt.y < r.Min.y) r.Min.y = pt.y;
        if (pt.x > r.Max.x) r.Max.x = pt.x;
        if (pt.y > r.Max.y) r.Max.y = pt.y;
    }
    return r;
}

//------------------------------------------------------------------------------
// Line intersection by sign-change detection
static inline ImCubicBezierIntersectResult ImCubicBezierLineIntersect(
    const ImVec2& p0, const ImVec2& p1,
    const ImVec2& p2, const ImVec2& p3,
    const ImVec2& a,  const ImVec2& b)
{
    ImCubicBezierIntersectResult result;
    result.Count = 0;
    const int N = 32;
    ImVec2 prev = ImCubicBezierSample(p0, p1, p2, p3, 0.0f);
    float dPrev = (b.x - a.x)*(prev.y - a.y) - (b.y - a.y)*(prev.x - a.x);
    for (int i = 1; i <= N && result.Count < 3; ++i)
    {
        float t = (float)i / N;
        ImVec2 cur = ImCubicBezierSample(p0, p1, p2, p3, t);
        float dCur = (b.x - a.x)*(cur.y - a.y) - (b.y - a.y)*(cur.x - a.x);
        if (dPrev * dCur < 0.0f)
        {
            float alpha = dPrev / (dPrev - dCur);
            ImVec2 pt(prev.x + alpha*(cur.x - prev.x), prev.y + alpha*(cur.y - prev.y));
            float lenSq = (b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y);
            if (lenSq > 1e-8f)
            {
                float proj = ((pt.x-a.x)*(b.x-a.x) + (pt.y-a.y)*(b.y-a.y)) / lenSq;
                if (proj >= 0.0f && proj <= 1.0f)
                    result.Points[result.Count++] = pt;
            }
        }
        prev  = cur;
        dPrev = dCur;
    }
    return result;
}

//------------------------------------------------------------------------------
// Adaptive subdivision (de Casteljau)
template<typename F>
static inline void ImCubicBezierSubdivide(
    F callback,
    const ImVec2& p0, const ImVec2& p1,
    const ImVec2& p2, const ImVec2& p3,
    float flatness = 0.5f, int depth = 0)
{
    // Flatness check
    float d1 = ImLength(ImVec2(p1.x - (p0.x*2.0f/3.0f + p3.x/3.0f),
                               p1.y - (p0.y*2.0f/3.0f + p3.y/3.0f)));
    float d2 = ImLength(ImVec2(p2.x - (p0.x/3.0f + p3.x*2.0f/3.0f),
                               p2.y - (p0.y/3.0f + p3.y*2.0f/3.0f)));
    if ((d1 + d2) <= flatness || depth >= 8)
    {
        ImCubicBezierSubdivideSample s;
        s.Point   = p3;
        s.Tangent = ImVec2(p3.x - p0.x, p3.y - p0.y);
        callback(s);
        return;
    }
    ImVec2 m01((p0.x+p1.x)*0.5f, (p0.y+p1.y)*0.5f);
    ImVec2 m12((p1.x+p2.x)*0.5f, (p1.y+p2.y)*0.5f);
    ImVec2 m23((p2.x+p3.x)*0.5f, (p2.y+p3.y)*0.5f);
    ImVec2 m012((m01.x+m12.x)*0.5f, (m01.y+m12.y)*0.5f);
    ImVec2 m123((m12.x+m23.x)*0.5f, (m12.y+m23.y)*0.5f);
    ImVec2 mid((m012.x+m123.x)*0.5f, (m012.y+m123.y)*0.5f);
    ImCubicBezierSubdivide(callback, p0, m01, m012, mid,  flatness, depth+1);
    ImCubicBezierSubdivide(callback, mid, m123, m23, p3, flatness, depth+1);
}

//------------------------------------------------------------------------------
// Project point onto cubic bezier, returns closest point and distance
static inline ImProjectResult ImProjectOnCubicBezier(
    const ImVec2& pt,
    const ImVec2& p0, const ImVec2& p1,
    const ImVec2& p2, const ImVec2& p3,
    int steps = 50)
{
    ImProjectResult best;
    best.Time     = 0.0f;
    best.Point    = p0;
    best.Distance = FLT_MAX;
    for (int i = 0; i <= steps; ++i)
    {
        float  t   = (float)i / steps;
        ImVec2 cur = ImCubicBezierSample(p0, p1, p2, p3, t);
        float  dx  = cur.x - pt.x;
        float  dy  = cur.y - pt.y;
        float  d   = ImSqrt(dx*dx + dy*dy);
        if (d < best.Distance)
        {
            best.Distance = d;
            best.Time     = t;
            best.Point    = cur;
        }
    }
    return best;
}

//------------------------------------------------------------------------------
// Fixed arc-length step sampling
template<typename F>
static inline void ImCubicBezierFixedStep(
    F callback,
    const ImCubicBezierPoints& curve,
    float step,
    bool  /*closed*/  = false,
    float /*overlap*/ = 0.5f,
    float /*error*/   = 0.001f)
{
    const int N = 128;
    float totalLen = 0.0f;
    float nextStep = step;
    ImVec2 prev = curve.P0;
    for (int i = 1; i <= N; ++i)
    {
        ImVec2 cur  = ImCubicBezierSample(curve.P0, curve.P1, curve.P2, curve.P3, (float)i / N);
        float  seg  = ImLength(cur - prev);
        totalLen   += seg;
        while (totalLen >= nextStep)
        {
            float  over  = totalLen - nextStep;
            float  alpha = (seg > 1e-6f) ? (1.0f - over / seg) : 1.0f;
            ImVec2 pt(prev.x + alpha*(cur.x - prev.x), prev.y + alpha*(cur.y - prev.y));
            ImCubicBezierFixedStepSample s;
            s.Length = nextStep;
            s.Point  = pt;
            callback(s);
            nextStep += step;
        }
        prev = cur;
    }
}

# endif // __IMGUI_BEZIER_MATH_H__
