#include "FadeObject.h"
#include "CDrawUtil.h"
#include "GameMain.h"
#include "MyImgui.h"
#include "ObjectManager.h"
#include "SceneManager.h"

//-----------------------------------------------------------------------------
// 初期化
FadeObject::FadeObject()
    : m_isFading(false), m_timer(0.0f), m_duration(1.0f), m_alpha(0.0f),
      m_isFadeOut(true), m_onComplete(nullptr), m_showLoading(false),
      m_sprite(nullptr), m_loadingImage(nullptr) 
{
  ObjectManager::SetDrawOrder(this, -1000);

  m_sprite = new CSprite();
  m_loadingImage = new CSpriteImage(GameDevice()->m_pShader);
  m_loadingImage->Load("Data/Image/Loading.png");
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 終了処理
FadeObject::~FadeObject() {
  if (m_sprite)
    delete m_sprite;
  if (m_loadingImage)
    delete m_loadingImage;

  if (GameDevice() && GameDevice()->m_pDI) {
    GameDevice()->m_pDI->SetInputEnabled(true);
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 更新
void FadeObject::Update() {
	//フェード中でなければ何もしない
    if (!m_isFading)
        return;

	// タイマーを進める
    float dt = SceneManager::UnscaledDeltaTime();
    m_timer += dt;

	// 経過時間に応じてアルファ値を計算する
    float t = m_timer / m_duration;
    if (t > 1.0f)
        t = 1.0f;

	// フェードアウトなら
    if (m_isFadeOut) {
        m_alpha = t;
    }
	// フェードインなら
    else {
        m_alpha = 1.0f - t;
    }

	// フェードが完了したら
    if (m_timer >= m_duration) {
        m_isFading = false;
        if (m_onComplete) {
            m_onComplete();
        }
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 描画
void FadeObject::Draw() {
    // フェード中でなければ何もしない
    if (m_alpha <= 0.0f)
        return;

    // アルファ値を0-255の範囲に変換
    int a = static_cast<int>(m_alpha * 255.0f);
    if (a > 255)
        a = 255;
    // 念のため0未満は0にする
    if (a < 0)
        a = 0;

    // 色をARGB形式で作成（黒の場合はRGBが0なので、アルファ値だけを設定すればOK）
    int color = (a << 24) | 0x000000;

    // 画面全体に半透明の矩形を描画
    CDrawUtil::DrawRect(0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, color);

    // ローディング画像の描画
    if (m_showLoading && m_loadingImage && m_sprite) {
        // 画面サイズを取得
        float sw = (float)WINDOW_WIDTH;
        float sh = (float)WINDOW_HEIGHT;

		// ローディング画像を描画するかどうかと、そのアルファ値を決定
        bool drawLoading = false;
        float loadingAlpha = 1.0f;

		// フェードアウト中は、アルファ値が0.99以上になったらローディング画像を完全に表示する
        if (m_isFadeOut) {
			// フェードアウト中は、アルファ値が0.99以上になったらローディング画像を完全に表示する
            if (m_alpha >= 0.99f) {
                drawLoading = true;
                loadingAlpha = 1.0f;
            }
        }
		// フェードイン中は、アルファ値が0.01以下になったらローディング画像を完全に表示する
        else {
            drawLoading = true;
            loadingAlpha = m_alpha;
        }

		// ローディング画像を描画する場合
        if (drawLoading) {
			// ローディング画像のテクスチャが有効なら描画する
            if (m_loadingImage->m_pTexture) {
				// ImGuiの描画リストを使って、ローディング画像を画面の右下に描画する
                ImTextureID texID = (ImTextureID)m_loadingImage->m_pTexture.Get();

				// ローディング画像のサイズを取得
                float imgW = (float)m_loadingImage->m_dwImageWidth;
                float imgH = (float)m_loadingImage->m_dwImageHeight;

				// 画面の右下に配置するための座標を計算する
                float padding = 20.0f;
                float x = sw - imgW - padding;
                float y = sh - imgH - padding;

				// 描画する矩形の左上と右下の座標を設定
                ImVec2 pMin(x, y);
                ImVec2 pMax(x + imgW, y + imgH);

				// ローディング画像のアルファ値を0-255の範囲に変換
                int alphaVal = (int)(loadingAlpha * 255.0f);
                if (alphaVal > 255)
                    alphaVal = 255;
				// 念のため0未満は0にする
                if (alphaVal < 0)
                    alphaVal = 0;

				// 色をARGB形式で作成（白の場合はRGBが255なので、アルファ値だけを設定すればOK）
                ImU32 col = IM_COL32(255, 255, 255, alphaVal);

				// ImGuiの描画リストを使って、ローディング画像を画面の右下に描画する
                ImGui::GetBackgroundDrawList()->AddImage(
                    texID, pMin, pMax, ImVec2(0, 0), ImVec2(1, 1), col);
            }
        }
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// フェードアウト開始
void FadeObject::StartFadeOut(float duration,
                              std::function<void()> onComplete) {
  m_duration = duration;
  m_onComplete = onComplete;
  m_isFadeOut = true;
  m_timer = 0.0f;
  m_alpha = 0.0f;
  m_isFading = true;

  // フェードアウト中は入力を無効にする
  if (GameDevice()->m_pDI) {
    GameDevice()->m_pDI->SetInputEnabled(false);
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// フェードイン開始
void FadeObject::StartFadeIn(float duration, std::function<void()> onComplete) {
	// フェードイン開始
    m_duration = duration;
	// フェードイン完了時のコールバックを設定。フェードイン完了後に入力を有効にするための処理もここで行う。
    m_onComplete = [onComplete]() {
		// フェードイン完了後に入力を有効にする
        if (onComplete)
            onComplete();
		// フェードイン完了後に入力を有効にする
        if (GameDevice()->m_pDI) {
            GameDevice()->m_pDI->SetInputEnabled(true);
        }
        };
    m_isFadeOut = false;
    m_timer = 0.0f;
    m_alpha = 1.0f;
    m_isFading = true;

}
//-----------------------------------------------------------------------------