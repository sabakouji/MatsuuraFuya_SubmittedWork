#include "StageSelectScene.h"
#include "AudioManager.h"
#include "CDrawUtil.h"
#include "DataCarrier.h"
#include "FadeObject.h"
#include "GameMain.h"
#include "ObjectManager.h"
#include "SceneManager.h"
#include <algorithm>

//-----------------------------------------------------------------------------
// 初期化
StageSelectScene::StageSelectScene()
    : m_selectIndex(0), m_scrollOffset(0), m_confirmExecute(false),
    m_selectedStageIndex(-1) {
	// ステージスクリプトの読み込み
    std::string searchPath = "Data/Script/*.txt";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

	// ファイルが見つかった場合、ループして処理
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
			// ディレクトリでない場合のみ処理
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string filename = findData.cFileName;
                int sortOrder = 9999;
                size_t sepPos = filename.find('_');
                if (sepPos != std::string::npos) {
                    std::string prefix = filename.substr(0, sepPos);
                    std::string numStr = "";
                    for (char c : prefix) {
                        if (c >= '0' && c <= '9') {
                            numStr += c;
                        }
                        else {
                            break;
                        }
                    }
                    if (!numStr.empty()) {
                        sortOrder = std::stoi(numStr);
                    }
                }

                size_t lastIndex = filename.find_last_of(".");
                if (lastIndex != std::string::npos) {
                    std::string name = filename.substr(0, lastIndex);
                    if (sortOrder != 9999 && sepPos != std::string::npos) {
                        if (sepPos + 1 < name.length()) {
                            name = name.substr(sepPos + 1);
                        }
                    }
                    m_stages.push_back({ name, "Data/Script/" + filename, sortOrder });
                }
            }
        } 

		// 次のファイルを検索
        while (FindNextFileA(hFind, &findData));
        FindClose(hFind);

        std::sort(m_stages.begin(), m_stages.end(),
            [](const StageData& a, const StageData& b) {
                return a.sortOrder < b.sortOrder;
            });
    }

	// スプライトの読み込み
    m_bgImage = new CSpriteImage(GameDevice()->m_pShader);
    m_bgImage->Load("Data/Image/SelectBG.png");

    m_bgSelectImage = new CSpriteImage(GameDevice()->m_pShader);
    m_bgSelectImage->Load("Data/Image/StageSelect_BackGround.png");

    m_btnStageImage = new CSpriteImage(GameDevice()->m_pShader);
    m_btnStageImage->Load("Data/Image/StageSelect_Button.png");

    m_btnStageCheckImage = new CSpriteImage(GameDevice()->m_pShader);
    m_btnStageCheckImage->Load("Data/Image/StageSelect_Button_CheckMark.png");

    m_btnAIEditImage = new CSpriteImage(GameDevice()->m_pShader);
    m_btnAIEditImage->Load("Data/Image/GotoEditor.png");

    m_tabImage = new CSpriteImage(GameDevice()->m_pShader);
    m_tabImage->Load("Data/Image/ExecTab.png");

    m_btnYesImage = new CSpriteImage(GameDevice()->m_pShader);
    m_btnYesImage->Load(
        "Data/Image/Exec_Button.png"); // Changed to Exec_Button based on context

    m_btnNoImage = new CSpriteImage(GameDevice()->m_pShader);
    m_btnNoImage->Load("Data/Image/ExitButton_Cancel.png");

    m_sprite = new CSprite();

	// フェードイン開始
    FadeObject* fade = ObjectManager::FindGameObject<FadeObject>();
    if (fade) {
        fade->StartFadeIn(0.5f, [fade]() {
            ObjectManager::DontDestroy(fade, false);
            ObjectManager::Destroy(fade);
            });
    }

    m_isCrickSEPlayed = false;

    AudioManager::StopAll();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 終了処理
StageSelectScene::~StageSelectScene() {
    if (m_bgImage)
        delete m_bgImage;
    if (m_bgSelectImage)
        delete m_bgSelectImage;
    if (m_btnStageImage)
        delete m_btnStageImage;
    if (m_btnStageCheckImage)
        delete m_btnStageCheckImage;
    if (m_btnAIEditImage)
        delete m_btnAIEditImage;

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
// 更新処理
void StageSelectScene::Update() {
	// シーン遷移の入力処理
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GameDevice()->m_pMain->m_hWnd, &pt);
    bool isClicked = GameDevice()->m_pDI->CheckMouse(KD_TRG, 0);

	// 画面サイズを取得
    float sw = (float)WINDOW_WIDTH;
    float sh = (float)WINDOW_HEIGHT;

	// 実行確認ダイアログの処理
    if (m_confirmExecute) {
		// ダイアログの位置は画面中央に配置
        float dw = m_tabImage ? (float)m_tabImage->m_dwImageWidth : 400.0f;
        float dh = m_tabImage ? (float)m_tabImage->m_dwImageHeight : 400.0f;
        float dx = (float)((int)(sw * 0.5f - dw * 0.5f));
        float dy = (float)((int)(sh * 0.5f - dh * 0.5f));

		// YESボタンの位置はポップアップの左下に配置
        float yesW = m_btnYesImage ? (float)m_btnYesImage->m_dwImageWidth : 100.0f;
        float yesH = m_btnYesImage ? (float)m_btnYesImage->m_dwImageHeight : 50.0f;
        float yesX = dx + 1;
        float yesY = dy + dh - 1; // Adjust this if button should be inside

		// NOボタンの位置はYESボタンの右隣に配置
        float noW = m_btnNoImage ? (float)m_btnNoImage->m_dwImageWidth : 100.0f;
        float noH = m_btnNoImage ? (float)m_btnNoImage->m_dwImageHeight : 50.0f;
        float noX = dx + yesW + 1;
        float noY = dy + dh - 1;

		// クリック音の再生（最初の1回だけ）
        if (m_isCrickSEPlayed == false) {
            AudioManager::Audio("Crick")->Play();
            m_isCrickSEPlayed = true;
        }

		// クリック処理
        if (isClicked) {
			// YESボタンがクリックされた場合、ゲームシーンへ遷移
            if (pt.x >= yesX && pt.x <= yesX + yesW && pt.y >= yesY &&
                pt.y <= yesY + yesH) {
                m_confirmExecute = false;
                FadeObject* fade = ObjectManager::CreateGameObject<FadeObject>();
                fade->SetShowLoading(true);
                ObjectManager::DontDestroy(fade);
                fade->StartFadeOut(1.0f, [this]() {
					// ステージスクリプトのパスをDataCarrierにセット
                    DataCarrier* dc = ObjectManager::FindGameObject<DataCarrier>();
                    if (dc && m_selectedStageIndex >= 0 &&
                        m_selectedStageIndex < (int)m_stages.size()) {
                        dc->SetScriptName(
                            m_stages[m_selectedStageIndex].scriptPath.c_str());
                    }
                    SceneManager::ChangeScene("PlayScene");
                    });
            }
			// NOボタンがクリックされた場合、ダイアログを閉じる
            if (pt.x >= noX && pt.x <= noX + noW && pt.y >= noY &&
                pt.y <= noY + noH) {
                m_confirmExecute = false;
                m_isCrickSEPlayed = false;
                m_selectedStageIndex = -1;
            }
            return;
        }
    }
	// AIエディタボタンの処理
    else {
        float aiW =
            m_btnAIEditImage ? (float)m_btnAIEditImage->m_dwImageWidth : 200.0f;
        float aiH =
            m_btnAIEditImage ? (float)m_btnAIEditImage->m_dwImageHeight : 100.0f;
        float aiX = sw - aiW;
        float aiY = 50.0f;

		// クリック処理
        if (isClicked) {
            if (pt.x >= aiX && pt.x <= aiX + aiW && pt.y >= aiY &&
                pt.y <= aiY + aiH) {
                SceneManager::ChangeScene("EditorScene");
                return;
            }
        }

		// ステージリストの処理
        float listX = sw * 0.3f;
        float listY = 200.0f;
        float itemW =
            m_btnStageImage ? (float)m_btnStageImage->m_dwImageWidth : 600.0f;
        float itemH =
            m_btnStageImage ? (float)m_btnStageImage->m_dwImageHeight : 100.0f;
        float gap = 20.0f;

		// クリック処理
        for (int i = 0; i < (int)m_stages.size(); i++) {
            float y = listY + (i) * (itemH + gap);
            float x = listX;

			// ステージアイテムがクリックされた場合、実行確認ダイアログを表示
            if (isClicked) {
                if (pt.x >= x && pt.x <= x + itemW && pt.y >= y && pt.y <= y + itemH) {
                    m_selectedStageIndex = i;
                    m_confirmExecute = true;
                }
            }
        }
    }

	// ESCキーでモード選択シーンに戻る
    if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_ESCAPE)) {
        SceneManager::ChangeScene("ModeSelectScene");
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 描画処理
void StageSelectScene::Draw() {
	// 画面サイズを取得
    float sw = (float)WINDOW_WIDTH;
    float sh = (float)WINDOW_HEIGHT;
    DataCarrier* dc = ObjectManager::FindGameObject<DataCarrier>();

	// 背景の描画
    if (m_bgSelectImage && m_sprite) {
        m_sprite->Draw(m_bgSelectImage, 0, 0, 0, 0, m_bgSelectImage->m_dwImageWidth,
            m_bgSelectImage->m_dwImageHeight);
    }
	// ベース背景は描画順の関係で後ろに配置
    else if (m_bgImage && m_sprite) {
        m_sprite->Draw(m_bgImage, 0, 0, 0, 0, m_bgImage->m_dwImageWidth,
            m_bgImage->m_dwImageHeight);
    }

	// AIエディタボタンの描画
    if (m_btnAIEditImage && m_sprite) {
        float aiW = (float)m_btnAIEditImage->m_dwImageWidth;
        float aiH = (float)m_btnAIEditImage->m_dwImageHeight;
        float aiX = sw - aiW;
        float aiY = 50.0f;
        m_sprite->Draw(m_btnAIEditImage, aiX, aiY, 0, 0, aiW, aiH);
    }

	// ステージリストの描画
    float listX = sw * 0.3f;
    float listY = 200.0f;

	// アイテムの幅はボタン画像の幅を基準にする
    float itemH =
        m_btnStageImage ? (float)m_btnStageImage->m_dwImageHeight : 100.0f;
    float gap = 20.0f;

	// 実行確認ダイアログと重ならないようにするためのポップアップの位置とサイズ
    float popX = 0, popY = 0, popW = 0, popH = 0;
    float btnZoneY = 0;

	//ダイアログが表示されている場合、ステージリストのテキストがダイアログやボタンと重ならないようにする
    if (m_confirmExecute) {
        popW = m_tabImage ? (float)m_tabImage->m_dwImageWidth : 400.0f;
        popH = m_tabImage ? (float)m_tabImage->m_dwImageHeight : 400.0f;
        popX = (float)((int)(sw * 0.5f - popW * 0.5f));
        popY = (float)((int)(sh * 0.5f - popH * 0.5f));

        float yesH = m_btnYesImage ? (float)m_btnYesImage->m_dwImageHeight : 50.0f;
        btnZoneY = popY + popH - 1; // Based on draw logic
    }

	// ステージリストの描画
    for (int i = 0; i < (int)m_stages.size(); i++) {
        float y = listY + (i) * (itemH + gap);
        float x = listX;

        bool isCleared = false;
        if (dc) {
            isCleared = dc->IsStageCleared(i);
        }

        CSpriteImage* pBtn = isCleared ? m_btnStageCheckImage : m_btnStageImage;
        if (!pBtn)
            pBtn = m_btnStageImage;

        if (pBtn && m_sprite) {
            m_sprite->Draw(pBtn, x, y, 0, 0, pBtn->m_dwImageWidth,
                pBtn->m_dwImageHeight);
        }

		// ダイアログやボタンと重ならないようにテキストの描画を制御
        bool overlapped = false;
        if (m_confirmExecute) {
            float tx = x + 150.0f;
            float ty = y + 30.0f;
            float tw = 200.0f; // Approximate text width
            float th = 30.0f;  // Approximate text height

			// テキストがポップアップと重なっているか、YES/NOボタンのエリアと重なっているかをチェック
            if (tx < popX + popW && tx + tw > popX && ty < popY + popH &&
                ty + th > popY) {
                overlapped = true;
            }

			// YES/NOボタンのエリアと重なっているかをチェック
            if (ty + th > btnZoneY &&
                ty < btnZoneY + 100.0f) { // +100 as arbitrary button height range
                overlapped = true;
            }
        }

		// テキストが重ならない場合のみ描画
        if (!overlapped) {
            CDrawUtil::DrawText(m_stages[i].name.c_str(), x + 150.0f, y + 30.0f,
                0xFFFFFFFF);
        }
    }

	// スクロールバーの描画（簡易的なもの）
    float itemW =
        m_btnStageImage ? (float)m_btnStageImage->m_dwImageWidth : 600.0f;
    float scrollX = listX + itemW + 30.0f;
    float scrollY = listY;
    float trackH = (itemH + gap) * 4;
    float trackW = 20.0f;
    CDrawUtil::DrawRect(scrollX, scrollY, trackW, trackH, 0xFF444444);

	// スクロールのサムの高さは、表示されているステージ数と全ステージ数の比率で決定
    int totalStages = (int)m_stages.size();
    int visible_stages = 4;
    float ratio = 1.0f;

	// 全ステージ数が表示可能なステージ数を超える場合のみ比率を計算
    if (totalStages > visible_stages) {
        ratio = (float)visible_stages / (float)totalStages;
    }

	// サムの高さはトラックの高さに比率を掛けたもの
    float thumbH = trackH * ratio;
    float thumbY = scrollY;

	// スクロールオフセットに応じてサムの位置を調整
    CDrawUtil::DrawRect(scrollX, thumbY, trackW, thumbH, 0xFFCCCCCC);

	// 実行確認ダイアログの描画
    if (m_confirmExecute) {
        float dw = 400.0f;
        float dh = 400.0f;

        if (m_tabImage) {
            dw = (float)m_tabImage->m_dwImageWidth;
            dh = (float)m_tabImage->m_dwImageHeight;
        }
        float dx = (float)((int)(sw * 0.5f - dw * 0.5f));
        float dy = (float)((int)(sh * 0.5f - dh * 0.5f));

		//タブ背景の描画
        if (m_tabImage && m_sprite)
            m_sprite->Draw(m_tabImage, dx, dy, 0, 0, dw, dh);

		// ステージ名テキストの描画（ボタンの上部、中央寄せ）
        if (m_selectedStageIndex >= 0 &&
            m_selectedStageIndex < (int)m_stages.size()) {
            CDrawUtil::DrawText(m_stages[m_selectedStageIndex].name.c_str(),
                dx + 70.0f, dy + (dh * 0.35f), 0xFFFFFFFF);
        }

		// YESボタンの位置はポップアップの左下に配置
        float yesW = 100.0f;
        float yesH = 50.0f;

		// YESボタンのサイズは画像のサイズを基準にする  
        if (m_btnYesImage) {
            yesW = (float)m_btnYesImage->m_dwImageWidth;
            yesH = (float)m_btnYesImage->m_dwImageHeight;
        }

		// YESボタンの位置はポップアップの左下に配置
        float yesX = dx + 1;
        float yesY = dy + dh - 1; // Bottom edge

		// YESボタンの描画
        if (m_btnYesImage && m_sprite)
            m_sprite->Draw(m_btnYesImage, yesX, yesY, 0, 0, yesW, yesH);

		// NOボタンの位置はYESボタンの右隣に配置
        float noW = 100.0f;
        float noH = 50.0f;

		// NOボタンのサイズは画像のサイズを基準にする
        if (m_btnNoImage) {
            noW = (float)m_btnNoImage->m_dwImageWidth;
            noH = (float)m_btnNoImage->m_dwImageHeight;
        }

		// NOボタンの位置はYESボタンの右隣に配置
        float noX = dx + yesW + 1;
        float noY = dy + dh - 1;

		// NOボタンの描画
        if (m_btnNoImage && m_sprite)
            m_sprite->Draw(m_btnNoImage, noX, noY, 0, 0, noW, noH);
    }
}
//-----------------------------------------------------------------------------