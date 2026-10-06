#include "Canvas.h"
#include "CDrawUtil.h"
#include <cmath>

//-----------------------------------------------------------------------------
// 初期化
Canvas::Canvas() {
  canvasimage = new CSpriteImage(GameDevice()->m_pShader);
  canvasimage->Load("Data/Image/GridLine_2.png");
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 終了処理
Canvas::~Canvas() {
  if (canvasimage) {
    delete canvasimage;
    canvasimage = nullptr;
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 描画
void Canvas::Draw(const VECTOR2 &offset, float zoom) {
  CSprite spr;
  // スクリーンサイズとテクスチャサイズを取得
  float screenW = (float)GameDevice()->m_pD3D->m_dwWindowWidth;
  float screenH = (float)GameDevice()->m_pD3D->m_dwWindowHeight;

  // テクスチャサイズを取得
  float texW = (float)canvasimage->m_dwImageWidth;
  float texH = (float)canvasimage->m_dwImageHeight;

  // オフセットをテクスチャサイズで割った余りを計算
  float srcX = std::fmod(offset.x, texW);
  float srcY = std::fmod(offset.y, texH);

  // 負の値の場合はテクスチャサイズを加算して正の値に変換
  if (srcX < 0)
    srcX += texW;
  if (srcY < 0)
    srcY += texH;

  // ズームに応じて描画するテクスチャの範囲を計算
  float srcW = screenW / zoom;
  float srcH = screenH / zoom;

  // テクスチャの範囲がテクスチャサイズを超えないように調整
  spr.Draw(canvasimage, 0, 0, (long)srcX, (long)srcY, (long)srcW, (long)srcH);
}
//-----------------------------------------------------------------------------