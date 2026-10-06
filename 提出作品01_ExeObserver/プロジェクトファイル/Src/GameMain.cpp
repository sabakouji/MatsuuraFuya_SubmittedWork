//=============================================================================
//		実習用ゲームプログラム                 ver 3.2        2023.1.31
//
//		ゲームのメイン処理
//																GameMain.cpp
//=============================================================================
#include "GameMain.h"
#include "AudioManager.h"
#include "ObjectManager.h"
#include "ResourceManager.h"
#include "SceneManager.h"


namespace {
CGameMain *gameMainInstance;
}

// ============================================================================================
//
// CGameMain ゲームメインクラスの実装
//
// ============================================================================================
//------------------------------------------------------------------------
//
//	ゲームメインクラスのコンストラクタ
//
//  引数　なし
//
//------------------------------------------------------------------------
CGameMain::CGameMain(CMain *pMain) {
  gameMainInstance = this;
  m_pMain = pMain; // メインクラス

  m_pD3D = nullptr;         //	Direct3Dオブジェクト
  m_pXAudio = nullptr;      //	XAudioオブジェクト
  m_pDI = nullptr;          //	DirectInputオブジェクト
  m_pFont = nullptr;        //	フォントテクスチャ
  m_pShader = nullptr;      //	シェーダー
  m_pFbxMeshCtrl = nullptr; // FBXメッシュコントロールクラス

  // カメラ・ライト・ビュー
  m_vEyePt = VECTOR3(0, 0, 0);    // カメラ（視点）位置
  m_vLookatPt = VECTOR3(0, 0, 0); // 注視位置
  m_mView = XMMatrixIdentity();   // ビューマトリックス
  m_mProj = XMMatrixIdentity();   // プロジェクションマトリックス
  m_vLightDir = VECTOR3(0, 0, 0); // ディレクション（平行）ライトの方向
}
//------------------------------------------------------------------------
//
//	ゲームメインクラスのデストラクタ
//
//------------------------------------------------------------------------
CGameMain::~CGameMain() {
  AudioManager::Release();
  ObjectManager::Release();
  SceneManager::Release();
  ResourceManager::Reset();
  MyImgui::ImguiQuit(); // -- 2020.11.15    // MyImguiの終了処理

  SAFE_DELETE(m_pFbxMeshCtrl); // -- 2021.2.4
  SAFE_DELETE(m_pShader);
  SAFE_DELETE(m_pFont);
  SAFE_DELETE(m_pDI);
  SAFE_DELETE(m_pXAudio);
  SAFE_DELETE(m_pD3D);

  CoUninitialize(); // COMの解放
}

//------------------------------------------------------------------------
//
//	アプリケーションの初期化処理
//
//  引数　なし
//
//	戻り値 HRESULT
//		S_OK	= 成功
//		E_FAIL	= 失敗
//
//------------------------------------------------------------------------
HRESULT CGameMain::Init() {
  // COM初期化
  // CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  CoInitialize(nullptr);

  // Direct3Dの初期化
  m_pD3D = new CDirect3D;
  if (FAILED(m_pD3D->InitD3D(m_pMain->m_hWnd, WINDOW_WIDTH, WINDOW_HEIGHT))) {
    MessageBox(0, _T("Direct3Dの初期化に失敗しました"), nullptr, MB_OK);
    return E_FAIL;
  }

  // シェーダーの初期化
  m_pShader = new CShader(m_pD3D);
  if (FAILED(m_pShader->InitShader())) {
    MessageBox(0, _T("Shaderの生成・初期化に失敗しました"), nullptr, MB_OK);
    return E_FAIL;
  }

  // XAudio2の初期化
  m_pXAudio = new CXAudio;
  if (FAILED(m_pXAudio->InitAudio(m_pMain->m_hWnd))) {
    MessageBox(0, _T("XAudio2の初期化に失敗しました"), nullptr, MB_OK);
    return E_FAIL;
  }

  // DirectInputの初期化
  m_pDI = new CDirectInput;
  if (!(m_pDI->StartDirectInput(m_pMain->m_hInstance, m_pMain->m_hWnd,
                                INIT_KEYBOARD | INIT_MOUSE | INIT_JOYSTICK,
                                WINDOW_WIDTH, WINDOW_HEIGHT))) {
    MessageBox(0, _T("DirectInputの初期化に失敗しました"), nullptr, MB_OK);
    return E_FAIL;
  }

  // フォントテクスチャの初期化
  m_pFont = new CFontTexture(m_pShader);

  // MyImguiの初期化
  MyImgui::ImguiInit(m_pMain->m_hWnd, m_pD3D, WINDOW_WIDTH,
                     WINDOW_HEIGHT); // -- 2020.11.15

  // Fbxメッシュコントロールクラス
  m_pFbxMeshCtrl = new CFbxMeshCtrl(m_pShader); // -- 2021.2.4

  // ----------------------------------------------------------------------------------------
  // 変数の初期化
  m_vLightDir = normalize(VECTOR3(0.8f, 1, -1)); // 平行光源の初期値。適宜変更。

  // プロジェッショントランスフォーム（投影変換）の初期値
  // m_mProj = XMMatrixPerspectiveFovLH((FLOAT)(XM_PI / 4), (FLOAT)WINDOW_WIDTH
  // / (FLOAT)WINDOW_HEIGHT, 0.1f, 1000.0);
  m_mProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(38.0f),
                                     (FLOAT)WINDOW_WIDTH / (FLOAT)WINDOW_HEIGHT,
                                     0.1f, 1000.0);

  // ビュートランスフォーム（視点座標変換）の初期値
  VECTOR3 vUpVec(0.0f, 1.0f, 0.0f); // 上方向
  m_vEyePt = VECTOR3(0.0f, 4.0f, -5.0f);
  m_vLookatPt = VECTOR3(0.0f, 2.0f, 0.0f);
  m_mView = XMMatrixLookAtLH(m_vEyePt, m_vLookatPt, vUpVec);

  // 各マネージャーの最初に1回だけ実行する処理
  AudioManager::Start();
  SceneManager::Start();
  ObjectManager::Start();

  // リフレッシュレートを設定する
  MainControl::UseFrameTimer(1.0f / 60.0f);

  return S_OK;
}

//------------------------------------------------------------------------
//
//	ゲームのループ処理
//
//  引数　なし
//
//	戻り値 なし
//
//------------------------------------------------------------------------
void CGameMain::Update() {
  MyImgui::ImguiNewFrame(); // MyImgui描画開始

  m_pDI->GetInput();  // 入力情報の取得
  m_pFont->Refresh(); // フォントのリフレッシュ

  if (m_pDI->CheckKey(KD_TRG, DIK_F4))
    ChangeScreenMode(); // フルスクリーンの切り替え       // -- 2018.12.14

  SceneManager::Update();
  ObjectManager::Update();
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
void CGameMain::GameMain() {}

void CGameMain::Draw() {
  // 画面クリア（実際は単色で画面を塗りつぶす処理）
  float ClearColor[4] = {0, 0, 0, 1};    // クリア色作成　RGBAの順
  m_pD3D->ClearRenderTarget(ClearColor); // 画面クリア

  SceneManager::Draw();
  ObjectManager::Draw();

  MyImgui::ImguiRender(); // MyImguiの描画処理    // -- 2020.11.15

  // 画面更新（バックバッファをフロントバッファに）
  m_pD3D->m_pSwapChain->Present(1, 0); // Vsync
  // m_pD3D->m_pSwapChain->Present(0, 0);                   // Vsyncなし
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
void CGameMain::Quit() { ; }

//------------------------------------------------------------------------    //
//-- 2018.12.14
//
//	ウィンドウモードとフルスクリーンモードを切り替える処理
//
//  引数　モード  0:ウィンドウモード 1:フルスクリーン
//  -1:モード切り替え（デフォルト値）
//
//	戻り値 なし
//
//------------------------------------------------------------------------
HRESULT CGameMain::ChangeScreenMode(int nMode) {
  HRESULT Ret = S_OK;
  BOOL bFullScreen; // フルスクリーンモードか？

  //	現在のスクリーンモードを得る
  m_pD3D->m_pSwapChain->GetFullscreenState(&bFullScreen, nullptr);

  switch (nMode) {
  case 0: // ウィンドウモードにする
    if (bFullScreen)
      Ret = m_pD3D->m_pSwapChain->SetFullscreenState(false, nullptr);
    break;
  case 1: // フルスクリーンモードにする
    if (!bFullScreen)
      Ret = m_pD3D->m_pSwapChain->SetFullscreenState(true, nullptr);
    break;
  default: // 現在のモードを切り替える
    if (bFullScreen) {
      Ret = m_pD3D->m_pSwapChain->SetFullscreenState(false, nullptr);
    } else {
      Ret = m_pD3D->m_pSwapChain->SetFullscreenState(true, nullptr);
    }
  }

  return Ret;
}

void CGameMain::SetWindowName(const char *name) {
  SetWindowText(m_pMain->m_hWnd, name);
}

CGameMain *GameDevice() { return gameMainInstance; }
