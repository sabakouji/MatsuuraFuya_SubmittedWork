//
//		ゲームメイン処理		                           ver 3.3        2024.5.20
//
#include "GameMain.h"
#include "SceneManager.h"
#include "ObjectManager.h"
#include "ResourceManager.h"
#include "AudioManager.h"

// ============================================================================================
//
// CGameMain ゲームメインクラスの処理
//
// ============================================================================================

namespace {
	CGameMain* gameMainInstance;
}

//------------------------------------------------------------------------
//
//	ゲームメインクラスのコンストラクタ	
//
//  引数　なし
//
//------------------------------------------------------------------------
CGameMain::CGameMain(CMain*	pMain)
{
	gameMainInstance = this;
	m_pMain = pMain;		// メインクラス

	m_pD3D = nullptr;			//	Direct3Dオブジェクト
	m_pXAudio = nullptr;		//	XAudioオブジェクト
	m_pDI = nullptr;			//	DirectInputDオブジェクト
	m_pFont = nullptr;			//	フォントテクスチャー
	m_pShader = nullptr;		//	シェーダー

	Scroll = VECTOR2(0, 0);		// スクロール位置

}
//------------------------------------------------------------------------
//
//	ゲームメインクラスのデストラクタ	
//
//------------------------------------------------------------------------
CGameMain::~CGameMain()
{
	AudioManager::Release();
	ObjectManager::Release();
	SceneManager::Release();
	ResourceManager::Reset();
	MyImgui::ImguiQuit();          // MyImguiの終了処理

	for (auto it = m_spriteImages.begin(); it != m_spriteImages.end(); ++it)
	{
		CSpriteImage* pSpriteImage = it->second;
		SAFE_DELETE(pSpriteImage);
	}
	m_spriteImages.clear();

	SAFE_DELETE(m_pShader);
	SAFE_DELETE(m_pFont);
	SAFE_DELETE(m_pDI);
	SAFE_DELETE(m_pXAudio);
	SAFE_DELETE(m_pD3D);
	

	CoUninitialize();   // COMの解放
}

//------------------------------------------------------------------------
//
//	アプリケーションの初期化処理	
//
//  引数　なし
//
//	戻り値 HRESULT
//		S_OK	= 正常
//		E_FAIL	= 異常
//
//------------------------------------------------------------------------
HRESULT CGameMain::Init()
{
	// ＣＯＭの初期化
	//CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	CoInitialize(nullptr);

	// Direct3Dの初期化
	m_pD3D = new CDirect3D;
	if (FAILED(m_pD3D->InitD3D(m_pMain->m_hWnd, WINDOW_WIDTH, WINDOW_HEIGHT))) {
		MessageBox(0, _T("Direct3Dを初期化出来ません"), nullptr, MB_OK);
		return E_FAIL;
	}

	// シェーダーの初期化
	m_pShader = new CShader(m_pD3D);
	if (FAILED(m_pShader->InitShader())) {
		MessageBox(0, _T("Shaderを生成・初期化出来ません"), nullptr, MB_OK);
		return E_FAIL;
	}

	// XAudio2の初期化
	m_pXAudio = new CXAudio;
	if (FAILED(m_pXAudio->InitAudio(m_pMain->m_hWnd))) {
		MessageBox(0, _T("XAudio2を初期化出来ません"), nullptr, MB_OK);
		return E_FAIL;
	}

	// DirectInputの初期化
	m_pDI = new CDirectInput;
	if (!(m_pDI->StartDirectInput(m_pMain->m_hInstance, m_pMain->m_hWnd, 
                    INIT_KEYBOARD | INIT_MOUSE | INIT_JOYSTICK, WINDOW_WIDTH, WINDOW_HEIGHT))) {
		MessageBox(0, _T("DirectInputを初期化出来ません"), nullptr, MB_OK);
		return E_FAIL;
	}

	// フォントテクスチャーの初期化
	m_pFont = new CFontTexture(m_pShader);

	// MyImguiの初期化
	MyImgui::ImguiInit(m_pMain->m_hWnd, m_pD3D, WINDOW_WIDTH, WINDOW_HEIGHT);

	// 各マネージャーの最初に１回だけ実行する処理
	AudioManager::Start();
	SceneManager::Start();
	ObjectManager::Start();

	return S_OK;
}


//------------------------------------------------------------------------
//
//	ゲームのループ処理  Update()/ Draw()	
//
//  引数　なし
//
//	戻り値 なし
//
//------------------------------------------------------------------------
void CGameMain::Update()
{
	MyImgui::ImguiNewFrame();   // MyImgui描画前処理

	m_pDI->GetInput();			// 入力情報の取得
	m_pFont->Refresh();         // フォント情報のリフレッシュ

	if (m_pDI->CheckKey(KD_TRG, DIK_F4))  ChangeScreenMode();   // フルスクリーンの切り替え

	// 各マネージャーの更新処理
	SceneManager::Update();
	ObjectManager::Update();
}

//---------------------
void CGameMain::Draw()
{
	//画面クリア（実際は単色で画面を塗りつぶす処理）
	float ClearColor[4] = { 0,0,0,1 };// クリア色作成　RGBAの順
	m_pD3D->ClearRenderTarget(ClearColor); // 画面クリア

	// 各マネージャーの描画処理
	ObjectManager::Draw();
	SceneManager::Draw();

	MyImgui::ImguiRender();      // MyImgui実描画処理

	//画面更新（バックバッファをフロントバッファに）
	//m_pD3D->m_pSwapChain->Present(1, 0);                   // 60fps Vsync
	m_pD3D->m_pSwapChain->Present(0, 0);                   // Vsyncなし
}

//------------------------------------------------------------------------
//
//	ゲームのメイン処理	
//
//  引数　なし
//
//	戻り値 なし
//
//------------------------------------------------------------------------
void CGameMain::GameMain()
{
}

//------------------------------------------------------------------------
//
//	アプリケーションの終了処理	
//
//  引数　なし
//
//	戻り値 なし
//
//------------------------------------------------------------------------
void CGameMain::Quit()
{
	;
}

//------------------------------------------------------------------------ 
//
//	ウィンドウモードとフルスクリーンモードを切り替える処理	
//
//  引数　モード  0:ウインドウモード　1:フルスクリーン　-1:モード切り替え（省略値）
//
//	戻り値 なし
//
//------------------------------------------------------------------------
HRESULT CGameMain::ChangeScreenMode(int nMode)
{
	HRESULT Ret = S_OK;
	BOOL bFullScreen;  // フルスクリーンモードか？

	//	現在のスクリーンモードを得る
	m_pD3D->m_pSwapChain->GetFullscreenState(&bFullScreen, nullptr);

	switch (nMode)
	{
	case 0:    // ウインドウモードにする
		if( bFullScreen ) Ret = m_pD3D->m_pSwapChain->SetFullscreenState(false, nullptr);
		break;
	case 1:    // フルスクリーンモードにする
		if( !bFullScreen ) Ret = m_pD3D->m_pSwapChain->SetFullscreenState(true, nullptr);
		break;
	default:   // 現在のモードを切り替える
		if (bFullScreen)
		{
			Ret = m_pD3D->m_pSwapChain->SetFullscreenState(false, nullptr);
		}
		else {
			Ret = m_pD3D->m_pSwapChain->SetFullscreenState(true, nullptr);
		}
	}

	return Ret;
}
CSpriteImage* CGameMain::GetSpriteImage(const char* filename)
{
	std::string key(filename);

	if (m_spriteImages.count(key)) {
		return m_spriteImages[key];
	}

	CSpriteImage* newImage = new CSpriteImage();

	if (SUCCEEDED(newImage->Load(filename)))
	{
		m_spriteImages[key] = newImage;
		return newImage;
	}
	else
	{
		delete newImage;
		return nullptr;
	}
}
// -----------------------------------------------------------------------
CGameMain* GameDevice()
{
	return gameMainInstance;
}
