#pragma once
#include "Object3D.h"
#include <functional>

/// <summary>
/// Fade Object
/// </summary>
class FadeObject : public Object3D {
public:
  FadeObject();
  ~FadeObject();

  void Update() override;
  void Draw() override;

  /// <summary>
  /// Start Fade Out
  /// </summary>
  /// <param name="duration">Duration (sec)</param>
  /// <param name="color">Color (default black)</param>
  void StartFadeOut(float duration, std::function<void()> onComplete = nullptr);

  /// <summary>
  /// Start Fade In
  /// </summary>
  void StartFadeIn(float duration, std::function<void()> onComplete = nullptr);

  bool IsFading() const { return m_isFading; }

private:
  bool m_isFading;
  float m_timer;
  float m_duration;
  float m_alpha;    // 0.0f (Transparent) ~ 1.0f (Opaque)
  bool m_isFadeOut; // true: FadeOut, false: FadeIn
  std::function<void()> m_onComplete;

  // Loading Image Support
  class CSprite *m_sprite;
  class CSpriteImage *m_loadingImage;
  bool m_showLoading;

public:
  void SetShowLoading(bool show) { m_showLoading = show; }
};
