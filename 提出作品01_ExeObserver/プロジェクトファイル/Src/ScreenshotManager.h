#pragma once
#include <d3d11.h>

class ScreenshotManager {
public:
  static void SaveScreenshot(struct ID3D11Texture2D *pTex);
};
