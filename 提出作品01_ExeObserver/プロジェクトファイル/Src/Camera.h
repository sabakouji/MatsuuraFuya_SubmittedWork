#pragma once
#include "Executor.h"
#include "Object3D.h"

#include <wrl/client.h>
// 前方宣言
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

class Camera : public Object3D {
public:
  Camera();
  ~Camera();
  void Update() override;
  void Draw() override;

private:
  Executor *executor;

  VECTOR3 lookPosition;

  float horizontalspeed;
  float verticalspeed;
  float inpX;
  float inpY;
  float inpZ;

  float yaw_;   // 左右
  float pitch_; // 上下

  float mouseSensitivity_; // マウス感度
  float cameraHeight_;     // プレイヤーからの高さ
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_ScreenshotTex;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ScreenshotSRV;
  float m_ThumbnailDisplayTime;
  bool hasScreenshot;
  int m_ScreenshotWidth;
  int m_ScreenshotHeight;
  bool m_screenshotRequested;

  void InputAction();
  void DrawThumbnail();
  void DrawFlash();
  void ReleaseScreenshotResources();

  int m_currentScore;
  char m_currentRank;
  float m_flashTimer;
  float m_flashDuration;

public:
  void TakeScreenshot(); // Public access for PlayScene

  int GetScore() const { return m_currentScore; }
  char GetRank() const { return m_currentRank; }
  bool HasScreenshot() const { return hasScreenshot; }
  bool IsScreenshotRequested() const { return m_screenshotRequested; }
};