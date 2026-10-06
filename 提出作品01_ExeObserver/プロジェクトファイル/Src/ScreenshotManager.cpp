#define _CRT_SECURE_NO_WARNINGS

#include "ScreenshotManager.h"
#include "Direct3D.h"
#include "GameMain.h"
#include <ctime>
#include <direct.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace {
// スクリーンショット保存先のフォルダ名
const char *SCREENSHOT_FOLDER = "ScreenShots";
} // namespace

void ScreenshotManager::SaveScreenshot(ID3D11Texture2D *pTex) {
  if (!pTex)
    return;

  CDirect3D *pD3D = GameDevice()->m_pD3D;
  if (!pD3D)
    return;

  ID3D11Device *pDevice = pD3D->m_pDevice.Get();
  ID3D11DeviceContext *pContext = pD3D->m_pDeviceContext.Get();

  if (!pDevice || !pContext)
    return;

  // フォルダ作成
  _mkdir(SCREENSHOT_FOLDER);

  D3D11_TEXTURE2D_DESC srcDesc;
  pTex->GetDesc(&srcDesc);

  // ステージングテクスチャの作成
  D3D11_TEXTURE2D_DESC stagingDesc = {};
  stagingDesc.Width = srcDesc.Width;
  stagingDesc.Height = srcDesc.Height;
  stagingDesc.MipLevels = 1;
  stagingDesc.ArraySize = 1;
  stagingDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  stagingDesc.SampleDesc.Count = 1;
  stagingDesc.SampleDesc.Quality = 0;
  stagingDesc.Usage = D3D11_USAGE_STAGING;
  stagingDesc.BindFlags = 0;
  stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  stagingDesc.MiscFlags = 0;

  ID3D11Texture2D *pStagingTex = nullptr;
  HRESULT hr = pDevice->CreateTexture2D(&stagingDesc, nullptr, &pStagingTex);
  if (FAILED(hr) || !pStagingTex)
    return;

  pContext->CopyResource(pStagingTex, pTex);

  // テクスチャのマッピング
  D3D11_MAPPED_SUBRESOURCE mappedResource;
  hr = pContext->Map(pStagingTex, 0, D3D11_MAP_READ, 0, &mappedResource);
  if (FAILED(hr)) {
    pStagingTex->Release();
    return;
  }

  // ファイル名の生成
  time_t now = time(nullptr);
  tm timeInfo;
  localtime_s(&timeInfo, &now);

  std::ostringstream filenameStream;
  filenameStream << SCREENSHOT_FOLDER << "/Screenshot_" << std::setfill('0')
                 << std::setw(4) << (timeInfo.tm_year + 1900) << std::setw(2)
                 << (timeInfo.tm_mon + 1) << std::setw(2) << timeInfo.tm_mday
                 << "_" << std::setw(2) << timeInfo.tm_hour << std::setw(2)
                 << timeInfo.tm_min << std::setw(2) << timeInfo.tm_sec
                 << ".png";

  int width = srcDesc.Width;
  int height = srcDesc.Height;
  int channels = 4; // RGBA
  int srcRowPitch = mappedResource.RowPitch;

  // 画像データのコピー
  unsigned char *imageData = new unsigned char[width * height * channels];
  unsigned char *srcData = static_cast<unsigned char *>(mappedResource.pData);

  for (int y = 0; y < height; ++y) {
    memcpy(imageData + (y * width * channels), srcData + (y * srcRowPitch),
           width * channels);
  }

  stbi_write_png(filenameStream.str().c_str(), width, height, channels,
                 imageData, width * channels);

  delete[] imageData;

  pContext->Unmap(pStagingTex, 0);
  pStagingTex->Release();
}
