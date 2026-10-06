#define _CRT_SECURE_NO_WARNINGS

#include "Camera.h"
#include "Direct3D.h"
#include "MapManager.h"
#include <ctime>
#include <d3d11.h>
#include <direct.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

// stb_image_write - シングルヘッダの画像保存ライブラリ

#include "DataCarrier.h"
#include "DisplayInfo.h" // Added include
#include "ObjectManager.h"
#include "ScoreCalculator.h"

namespace {
const float MOUSE_SENSITIVITY = 0.002f;             // マウス感度（調整用）
const float MIN_PITCH = XMConvertToRadians(-89.0f); // 見上げ/見下げの制限
const float MAX_PITCH = XMConvertToRadians(89.0f);
const float CAMERA_HEIGHT = 1.6f; // プレイヤーからの高さ
const float HORIZONTALBASESPEED = 0.08f;
const float VERTICALBASESPEED = 0.05f;
const float THUMBNAIL_DISPLAY_DURATION = 3.0f; // サムネイル表示時間
const float THUMBNAIL_FADEOUT_DURATION = 0.5f; // サムネイルフェードアウト時間
const float THUMBNAIL_WIDTH = 400.0f;          // サムネイル幅
const float THUMBNAIL_MARGIN = 40.0f;          // サムネイル余白

const float FLASH_DURATION = 0.45f; // フラッシュ時間

const float CAMERA_RADIUS = 0.5f; // 壁との当たり判定に使うカメラの半径
} // namespace

Camera::Camera() {
  ObjectManager::SetVisible(this, false); // 自体は表示しない
  SetPriority(-10000);                    // 最後に処理する

  executor = ObjectManager::FindGameObject<Executor>();

  yaw_ = 0.0f;
  pitch_ = 0.0f;
  mouseSensitivity_ = MOUSE_SENSITIVITY;
  cameraHeight_ = CAMERA_HEIGHT;
  horizontalspeed = HORIZONTALBASESPEED;
  verticalspeed = VERTICALBASESPEED;
  transform.position =
      executor->Position() + VECTOR3(0, cameraHeight_ + 0.2f, -2.5f);
  inpX = 0.0f;
  inpY = 0.0f;
  inpZ = 0.0f;

  m_ScreenshotTex = nullptr;
  m_ScreenshotSRV = nullptr;
  m_ThumbnailDisplayTime = 0.0f;
  hasScreenshot = false;
  m_ScreenshotWidth = 0;
  m_ScreenshotHeight = 0;
  m_screenshotRequested = false;

  m_flashTimer = 0.0f;
  m_flashDuration = FLASH_DURATION;
}

Camera::~Camera() { ReleaseScreenshotResources(); }

void Camera::ReleaseScreenshotResources() {
  m_ScreenshotSRV.Reset();
  m_ScreenshotTex.Reset();
  hasScreenshot = false;
}

void Camera::Update() {
  if (!executor)
    return;

  // Check Pause State
  DisplayInfo *di = ObjectManager::FindGameObject<DisplayInfo>();
  if (di && di->IsPaused()) {
    return; // Stop Camera update if paused
  }

  // Check Game Over State
  if (executor && executor->HpRatio() <= 0.0f) {
    return; // Stop Camera update if dead
  }

  InputAction();

  if (m_ThumbnailDisplayTime > 0.0f) {
    m_ThumbnailDisplayTime -= 1.0f / 60.0f;
  }

  if (m_flashTimer > 0.0f) {
    m_flashTimer -= 1.0f / 60.0f;
  }

  DIMOUSESTATE ms = GameDevice()->m_pDI->GetMouseState();

  LONG dx = ms.lX;
  LONG dy = ms.lY;

  yaw_ += dx * mouseSensitivity_;
  pitch_ += -dy * mouseSensitivity_;

  if (pitch_ > MAX_PITCH)
    pitch_ = MAX_PITCH;
  if (pitch_ < MIN_PITCH)
    pitch_ = MIN_PITCH;

  // 描画に使う視点は transform.position からずらした位置にある。
  // 壁との当たり判定もこの視点位置で行う（原点で判定すると視点だけ壁にめり込む）
  const VECTOR3 eyeOffset = VECTOR3{0, cameraHeight_ + 0.2f, -2.5f};
  VECTOR3 eye = transform.position + eyeOffset;

  float cosPitch = cosf(pitch_);
  float sinPitch = sinf(pitch_);
  float cosYaw = cosf(yaw_);
  float sinYaw = sinf(yaw_);

  VECTOR3 forward(sinYaw * cosPitch, // x
                  sinPitch,          // y
                  cosYaw * cosPitch  // z
  );

  VECTOR3 look = eye + forward;

  lookPosition = look;

  GameDevice()->m_vEyePt = transform.position; // カメラ座標
  GameDevice()->m_vLookatPt = look;            // 注視点
  GameDevice()->m_mView = XMMatrixLookAtLH(eye, look, VECTOR3(0, 1, 0));

  std::list<Object3D *> objList = ObjectManager::FindGameObjects<Object3D>();
  for (Object3D *&obj : objList) {
    if (obj != this) {
      float distQ = magnitudeSQ(obj->Position() - transform.position);
      ObjectManager::SetEyeDist(obj, distQ);
    }
  }

  VECTOR3 forwardXZ(forward.x, 0.0f, forward.z);

  if (forwardXZ.Length() > 0.0f) {
    forwardXZ = XMVector3Normalize(forwardXZ);
  }

  VECTOR3 rightXZ(forwardXZ.z, 0.0f, -forwardXZ.x);

  if (rightXZ.Length() > 0.0f) {
    rightXZ = XMVector3Normalize(rightXZ);
  }

  VECTOR3 moveHorizontal = forwardXZ * inpZ + rightXZ * inpX;

  if (moveHorizontal.Length() > 0.0f) {
    moveHorizontal = XMVector3Normalize(moveHorizontal) * horizontalspeed;
  }

  VECTOR3 moveVertical(0.0f, inpY * verticalspeed, 0.0f);

  VECTOR3 velocity = moveHorizontal + moveVertical;

  // マップとの当たり判定を行い、壁を抜けないようにする。
  // マップの衝突形状は各オブジェクトのmeshColではなくMapManagerに集約されているため、
  // キャラクターと同じくMapManager経由で判定する。
  // カメラは自由移動なので重力は加味しない（IsCollisionMoveGravityではない）。
  VECTOR3 nextEye = eye + velocity;

  MapManager *mm = ObjectManager::FindGameObject<MapManager>();
  if (mm != nullptr) {
    // 衝突していた場合、nextEyeが壁の外へ押し出される
    mm->IsCollisionMove(eye, nextEye, CAMERA_RADIUS);
  }

  // 押し出された視点位置から自身の座標を逆算する
  transform.position = nextEye - eyeOffset;

  DrawThumbnail();
  DrawFlash();
}

void Camera::Draw() {}

void Camera::InputAction() {
  // 移動入力
  if (GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_W)) {
    // 前進
    inpZ = 1.0f;
  } else if (GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_S)) {
    // 後退
    inpZ = -1.0f;
  } else {
    // 停止
    inpZ = 0.0f;
  }

  if (GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_D)) {
    // 右移動
    inpX = 1.0f;
  } else if (GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_A)) {
    // 左移動
    inpX = -1.0f;
  } else {
    // 停止
    inpX = 0.0f;
  }

  if (GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_SPACE)) {
    // 浮上
    inpY = 1.0f;
  } else if (GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_LCONTROL)) {
    // 降下
    inpY = -1.0f;
  } else {
    // 停止
    inpY = 0.0f;
  }

  // スクリーンショット撮影
  if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_F)) {
    m_screenshotRequested = true;
  }
}

void Camera::TakeScreenshot() {
  CDirect3D *pD3D = GameDevice()->m_pD3D;
  if (!pD3D)
    return;

  ID3D11Device *pDevice = pD3D->m_pDevice.Get();
  ID3D11DeviceContext *pContext = pD3D->m_pDeviceContext.Get();
  IDXGISwapChain *pSwapChain = pD3D->m_pSwapChain.Get();

  if (!pDevice || !pContext || !pSwapChain)
    return;

  // 既存のリソースを解放
  ReleaseScreenshotResources();

  // 既存のリソースを解放
  ReleaseScreenshotResources();

  // 現在のアクティブなレンダーターゲットを取得
  ID3D11RenderTargetView *pRTV = nullptr;
  pContext->OMGetRenderTargets(1, &pRTV, nullptr);

  ID3D11Texture2D *pBackBufferTex = nullptr; // Declare here
  HRESULT hr = S_OK;

  if (!pRTV) {
    // RTVが取得できない場合（ありえないはずだが）、スワップチェーンからバックアップ
    // あるいはエラーとして帰る
    // fallback to swapchain logic just in case?
    // For now, let's assume RTV is set. If not, we can't capture what's drawn.
    pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                          (LPVOID *)&pBackBufferTex);
  } else {
    // RTVからリソース（テクスチャ）を取得
    ID3D11Resource *pRes = nullptr;
    pRTV->GetResource(&pRes);
    pRTV->Release(); // GetRenderTargetsでAddRefされるのでRelease

    if (pRes) {
      pRes->QueryInterface(__uuidof(ID3D11Texture2D), (void **)&pBackBufferTex);
      pRes->Release();
    }
  }

  if (!pBackBufferTex)
    return;

  // バックバッファの情報を取得
  D3D11_TEXTURE2D_DESC backBufferDesc;
  pBackBufferTex->GetDesc(&backBufferDesc);

  m_ScreenshotWidth = backBufferDesc.Width;
  m_ScreenshotHeight = backBufferDesc.Height;

  // スクリーンショット用のテクスチャを作成
  D3D11_TEXTURE2D_DESC screenshotDesc = {};
  screenshotDesc.Width = backBufferDesc.Width;
  screenshotDesc.Height = backBufferDesc.Height;
  screenshotDesc.MipLevels = 1;
  screenshotDesc.ArraySize = 1;
  screenshotDesc.Format =
      DXGI_FORMAT_R8G8B8A8_UNORM; // RTVがHDR(R16G16B16A16)などの場合、コピー時にフォーマット変換が必要かも？
                                  // CopyResourceは同一フォーマットでないと失敗する.
                                  // RTVのフォーマットを確認すべき.

  if (backBufferDesc.Format != screenshotDesc.Format) {
    // フォーマットが違う場合（例：R8G8B8A8_UNORM_SRGB vs UNORM）、
    // CopyResourceは厳密な一致を要求しない場合もあるが（Typelessなど）、
    // 基本的にはResolveかCopySubresourceRegionを使う。
    // ここでは簡略化のため、作成するテクスチャの方をバックバッファのフォーマットに合わせる。
    screenshotDesc.Format = backBufferDesc.Format;
  }

  screenshotDesc.SampleDesc.Count = 1;
  screenshotDesc.SampleDesc.Quality = 0;
  screenshotDesc.Usage = D3D11_USAGE_DEFAULT;
  screenshotDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  screenshotDesc.CPUAccessFlags = 0;
  screenshotDesc.MiscFlags = 0;

  hr = pDevice->CreateTexture2D(&screenshotDesc, nullptr,
                                m_ScreenshotTex.ReleaseAndGetAddressOf());

  if (FAILED(hr)) {
    pBackBufferTex->Release();
    return;
  }

  if (backBufferDesc.SampleDesc.Count > 1) {
    D3D11_TEXTURE2D_DESC resolveDesc = backBufferDesc;
    resolveDesc.SampleDesc.Count = 1;
    resolveDesc.SampleDesc.Quality = 0;
    resolveDesc.Usage = D3D11_USAGE_DEFAULT;
    resolveDesc.BindFlags = 0;
    resolveDesc.CPUAccessFlags = 0;
    resolveDesc.MiscFlags = 0;

    ID3D11Texture2D *pResolvedTex = nullptr;
    // Resolve用の一時テクスチャ作成（フォーマットは合わせる）
    hr = pDevice->CreateTexture2D(&resolveDesc, nullptr, &pResolvedTex);
    if (SUCCEEDED(hr) && pResolvedTex) {
      pContext->ResolveSubresource(pResolvedTex, 0, pBackBufferTex, 0,
                                   backBufferDesc.Format);
      pContext->CopyResource(m_ScreenshotTex.Get(), pResolvedTex);
      pResolvedTex->Release();
    }
  } else {
    // マルチサンプリングされていない場合は、コピーする
    pContext->CopyResource(m_ScreenshotTex.Get(), pBackBufferTex);
  }

  pBackBufferTex->Release();

  // シェーダーリソースビューを作成
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Texture2D.MipLevels = 1;

  hr = pDevice->CreateShaderResourceView(m_ScreenshotTex.Get(), &srvDesc,
                                         m_ScreenshotSRV.ReleaseAndGetAddressOf());
  if (FAILED(hr)) {
    ReleaseScreenshotResources();
    return;
  }

  hasScreenshot = true;
  m_ThumbnailDisplayTime = THUMBNAIL_DISPLAY_DURATION;

  int score = 0;
  char rank = 'E';
  ScoreCalculator::CalculateScore(executor, this, score, rank);

  m_currentScore = score;
  m_currentRank = rank;
  std::cout << "PHOTO TAKEN! Score: " << score << " Rank: " << rank
            << std::endl;

  DataCarrier *dc = ObjectManager::FindGameObject<DataCarrier>();
  if (dc) {
    DataCarrier::ScreenshotData data;
    data.score = score;
    data.rank = rank;
    data.texture = m_ScreenshotSRV.Get();
    data.rawTexture = m_ScreenshotTex.Get();
    data.width = m_ScreenshotWidth;
    data.height = m_ScreenshotHeight;

    if (data.texture)
      data.texture->AddRef();
    if (data.rawTexture)
      data.rawTexture->AddRef();

    dc->AddScreenshot(data);
  }

  m_screenshotRequested = false; // Reset request flag

  m_flashTimer = m_flashDuration;
}

void Camera::DrawThumbnail() {
  if (!hasScreenshot || !m_ScreenshotSRV || m_ThumbnailDisplayTime <= 0.0f)
    return;

  // アルファ値の計算
  float alpha = 1.0f;
  if (m_ThumbnailDisplayTime < THUMBNAIL_FADEOUT_DURATION) {
    alpha = m_ThumbnailDisplayTime / THUMBNAIL_FADEOUT_DURATION;
  }

  // サムネイルのサイズ
  float aspectRatio = (float)m_ScreenshotHeight / (float)m_ScreenshotWidth;
  float thumbnailWidth = THUMBNAIL_WIDTH;
  float thumbnailHeight = THUMBNAIL_WIDTH * aspectRatio;

  // サムネイルの位置
  float screenWidth = (float)WINDOW_WIDTH;
  float screenHeight = (float)WINDOW_HEIGHT;
  float posX = screenWidth - thumbnailWidth - THUMBNAIL_MARGIN;
  float posY = screenHeight - thumbnailHeight - THUMBNAIL_MARGIN;

  ImDrawList *list = ImGui::GetForegroundDrawList();

  // 背景の描画
  ImU32 borderColor = IM_COL32(0, 0, 0, (int)(alpha * 200));
  list->AddRectFilled(
      ImVec2(posX - 2.0f, posY - 2.0f),
      ImVec2(posX + thumbnailWidth + 2.0f, posY + thumbnailHeight + 2.0f),
      borderColor, 4.0f);

  // サムネイルの描画
  ImU32 tintColor = IM_COL32(255, 255, 255, (int)(alpha * 255));
  list->AddImage((ImTextureID)m_ScreenshotSRV.Get(), ImVec2(posX, posY),
                 ImVec2(posX + thumbnailWidth, posY + thumbnailHeight),
                 ImVec2(0, 0), ImVec2(1, 1), tintColor);
}

void Camera::DrawFlash() {
  if (m_flashTimer <= 0.0f)
    return;

  float alpha = m_flashTimer / m_flashDuration;
  if (alpha > 1.0f)
    alpha = 1.0f;
  if (alpha < 0.0f)
    alpha = 0.0f;

  ImDrawList *list = ImGui::GetForegroundDrawList();
  ImU32 color = IM_COL32(255, 255, 255, (int)(alpha * 255));

  list->AddRectFilled(ImVec2(0, 0),
                      ImVec2((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT), color);
}