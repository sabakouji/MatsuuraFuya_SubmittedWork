#include "ModeSelectScene.h"
#include "AudioManager.h"
#include "CDrawUtil.h"
#include "GameMain.h"
#include "SceneManager.h"

//-----------------------------------------------------------------------------
// 初期化
ModeSelectScene::ModeSelectScene() : m_confirmExit(false) {
    // Init Sprites
    m_bgImage = new CSpriteImage(GameDevice()->m_pShader);
    m_bgImage->Load("Data/Image/SelectBG.png");

    m_imgStage = new CSpriteImage(GameDevice()->m_pShader);
    m_imgStage->Load("Data/Image/Stage.png");

    m_imgEditor = new CSpriteImage(GameDevice()->m_pShader);
    m_imgEditor->Load("Data/Image/AIEditor.png");

    m_imgExit = new CSpriteImage(GameDevice()->m_pShader);
    m_imgExit->Load("Data/Image/Exit_Button.png");

    m_imgBar = new CSpriteImage(GameDevice()->m_pShader);
    m_imgBar->Load("Data/Image/FrontBar.png");

    m_tabImage = new CSpriteImage(GameDevice()->m_pShader);
    m_tabImage->Load("Data/Image/ExitTab.png");

    m_btnYesImage = new CSpriteImage(GameDevice()->m_pShader);
    m_btnYesImage->Load("Data/Image/ExitButton_Exit.png");

    m_btnNoImage = new CSpriteImage(GameDevice()->m_pShader);
    m_btnNoImage->Load("Data/Image/ExitButton_Cancel.png");

    m_sprite = new CSprite();

    AudioManager::StopAll();
    AudioManager::Audio("BGM_Lobby")->Play(AUDIO_LOOP);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 終了処理
ModeSelectScene::~ModeSelectScene() {
    if (m_bgImage)
        delete m_bgImage;
    if (m_imgStage)
        delete m_imgStage;
    if (m_imgEditor)
        delete m_imgEditor;
    if (m_imgExit)
        delete m_imgExit;
    if (m_imgBar)
        delete m_imgBar;

    if (m_tabImage)
        delete m_tabImage;
    if (m_btnYesImage)
        delete m_btnYesImage;
    if (m_btnNoImage)
        delete m_btnNoImage;
    if (m_sprite)
        delete m_sprite;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void ModeSelectScene::Update() {
    float sw = (float)WINDOW_WIDTH;
    float sh = (float)WINDOW_HEIGHT;

	// シーン遷移の入力処理
    if (m_confirmExit) {
        if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_ESCAPE)) {
            m_confirmExit = false;
            return;
        }

		// マウスクリックの処理
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(GameDevice()->m_pMain->m_hWnd, &pt);
        bool isClicked = GameDevice()->m_pDI->CheckMouse(KD_TRG, 0);

		// ポップアップの位置は画面中央に配置
        float dw = 400.0f;
        float dh = 400.0f;
        if (m_tabImage) {
            dw = (float)m_tabImage->m_dwImageWidth;
            dh = (float)m_tabImage->m_dwImageHeight;
        }
        float dx = sw * 0.5f - dw * 0.5f;
        float dy = sh * 0.5f - dh * 0.5f;

		// YESボタンの位置はポップアップの左下に配置
        float yesW = 100.0f;
        float yesH = 50.0f;
        if (m_btnYesImage) {
            yesW = (float)m_btnYesImage->m_dwImageWidth;
            yesH = (float)m_btnYesImage->m_dwImageHeight;
        }
        float yesX = dx + 1;
        float yesY = dy + dh - 1;

		// NOボタンの位置はYESボタンの右隣に配置
        float noW = 100.0f;
        float noH = 50.0f;
        if (m_btnNoImage) {
            noW = (float)m_btnNoImage->m_dwImageWidth;
            noH = (float)m_btnNoImage->m_dwImageHeight;
        }
        float noX = dx + yesW + 1;
        float noY = dy + dh - 1;

		// クリック処理
        if (isClicked) {
            if (pt.x >= yesX && pt.x <= yesX + yesW && pt.y >= yesY &&
                pt.y <= yesY + yesH) {
                SceneManager::Exit();
            }
            if (pt.x >= noX && pt.x <= noX + noW && pt.y >= noY &&
                pt.y <= noY + noH) {
                m_confirmExit = false;
            }
        }
        return;
    }

	// シーン遷移の入力処理
    if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_ESCAPE)) {
        m_confirmExit = true;
    }

	// マウスクリックの処理
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GameDevice()->m_pMain->m_hWnd, &pt);
    bool isClicked = GameDevice()->m_pDI->CheckMouse(KD_TRG, 0);

	// ボタンの位置は画面中央に配置
    float stageW = 400.0f;
    float stageH = 300.0f;
    if (m_imgStage) {
        stageW = (float)m_imgStage->m_dwImageWidth;
        stageH = (float)m_imgStage->m_dwImageHeight;
    }

	// ボタンの位置は画面中央に配置
    float editorW = 400.0f;
    float editorH = 300.0f;
    if (m_imgEditor) {
        editorW = (float)m_imgEditor->m_dwImageWidth;
        editorH = (float)m_imgEditor->m_dwImageHeight;
    }

	// ボタン間のスペース
    float gap = 50.0f;
    float totalW = stageW + editorW + gap;
    float startX = (sw - totalW) / 2.0f;
    float startY = (sh - stageH) / 2.0f;

	// 各ボタンの位置を計算
    float stageX = startX;
    float stageY = (sh - stageH) / 2.0f;

	// EditorボタンはStageボタンの右側に配置
    float editorX = startX + stageW + gap;
    float editorY = (sh - editorH) / 2.0f;

	// Exitボタンは画面右下に配置
    float exitW = 100.0f;
    float exitH = 100.0f;
    if (m_imgExit) {
        exitW = (float)m_imgExit->m_dwImageWidth;
        exitH = (float)m_imgExit->m_dwImageHeight;
    }
    float exitX = sw - exitW - 20;
    float exitY = sh - exitH - 20;

	// クリック処理
    if (isClicked) {
        if (pt.x >= stageX && pt.x <= stageX + stageW && pt.y >= stageY &&
            pt.y <= stageY + stageH) {
            SceneManager::ChangeScene("StageSelectScene");
        }
        if (pt.x >= editorX && pt.x <= editorX + editorW && pt.y >= editorY &&
            pt.y <= editorY + editorH) {
            SceneManager::ChangeScene("EditorScene");
        }
        if (pt.x >= exitX && pt.x <= exitX + exitW && pt.y >= exitY &&
            pt.y <= exitY + exitH) {
            m_confirmExit = true;
        }
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 描画
void ModeSelectScene::Draw() {
	// 画面サイズ
    float sw = (float)WINDOW_WIDTH;
    float sh = (float)WINDOW_HEIGHT;

	// 背景の描画
    if (m_bgImage && m_sprite) {
        m_sprite->Draw(m_bgImage, 0, 0, 0, 0, m_bgImage->m_dwImageWidth,
            m_bgImage->m_dwImageHeight);
    }

	// 前面のバーの描画
    if (m_imgBar && m_sprite) {
        float barH = (float)m_imgBar->m_dwImageHeight;
        float barY = sh - barH;
        m_sprite->Draw(m_imgBar, 0, barY, 0, 0, m_imgBar->m_dwImageWidth,
            m_imgBar->m_dwImageHeight);
    }

	// ボタンの位置は画面中央に配置
    float stageW = 400.0f;
    float stageH = 300.0f;
    if (m_imgStage) {
        stageW = (float)m_imgStage->m_dwImageWidth;
        stageH = (float)m_imgStage->m_dwImageHeight;
    }

	// ボタンの位置は画面中央に配置
    float editorW = 400.0f;
    float editorH = 300.0f;
    if (m_imgEditor) {
        editorW = (float)m_imgEditor->m_dwImageWidth;
        editorH = (float)m_imgEditor->m_dwImageHeight;
    }

	// ボタン間のスペース
    float gap = 50.0f;
    float totalW = stageW + editorW + gap;
    float startX = (sw - totalW) / 2.0f;

	// 各ボタンの位置を計算
    float stageX = startX - gap;
    float stageY = (sh - stageH) / 2.0f;

	// EditorボタンはStageボタンの右側に配置
    float editorX = startX + stageW + gap * 2.0f;
    float editorY = (sh - editorH) / 2.0f;

	// Exitボタンは画面右下に配置
    if (m_imgStage && m_sprite) {
        m_sprite->Draw(m_imgStage, stageX, stageY, 0, 0, stageW, stageH);
    }

	// Editorボタンの描画
    if (m_imgEditor && m_sprite) {
        m_sprite->Draw(m_imgEditor, editorX, editorY, 0, 0, editorW, editorH);
    }

	// Exitボタンの描画
    if (m_imgExit && m_sprite) {
        float ew = (float)m_imgExit->m_dwImageWidth;
        float eh = (float)m_imgExit->m_dwImageHeight;
        float ex = sw - ew - 20;
        float ey = sh - eh - 20;
        m_sprite->Draw(m_imgExit, ex, ey, 0, 0, ew, eh);
    }

	// 確認ポップアップの描画
    if (m_confirmExit) {
		// ポップアップの位置は画面中央に配置
        float dw = 400.0f;
        float dh = 400.0f;
        if (m_tabImage) {
            dw = (float)m_tabImage->m_dwImageWidth;
            dh = (float)m_tabImage->m_dwImageHeight;
        }

		// ポップアップの位置は画面中央に配置
        float dx = (float)((int)(sw * 0.5f - dw * 0.5f));
        float dy = (float)((int)(sh * 0.5f - dh * 0.5f));

		//タブ背景の描画
        if (m_tabImage && m_sprite)
            m_sprite->Draw(m_tabImage, dx, dy, 0, 0, dw, dh);

		// ステージ名テキストの描画（ボタンの上部、中央寄せ）
        float yesW = 100.0f;
        float yesH = 50.0f;
        if (m_btnYesImage) {
            yesW = (float)m_btnYesImage->m_dwImageWidth;
            yesH = (float)m_btnYesImage->m_dwImageHeight;
        }

		// YESボタンの位置はポップアップの左下に配置
        float yesX = dx + 1;
        float yesY = dy + dh - 1;

		// YESボタンの描画
        if (m_btnYesImage)
            m_sprite->Draw(m_btnYesImage, yesX, yesY, 0, 0, yesW, yesH);

		// NOボタンの位置はYESボタンの右隣に配置
        float noW = 100.0f;
        float noH = 50.0f;
        if (m_btnNoImage) {
            noW = (float)m_btnNoImage->m_dwImageWidth;
            noH = (float)m_btnNoImage->m_dwImageHeight;
        }

		// NOボタンの位置はYESボタンの右隣に配置
        float noX = dx + yesW + 1;
        float noY = dy + dh - 1;

		// NOボタンの描画
        if (m_btnNoImage)
            m_sprite->Draw(m_btnNoImage, noX, noY, 0, 0, noW, noH);
    }
}
//-----------------------------------------------------------------------------