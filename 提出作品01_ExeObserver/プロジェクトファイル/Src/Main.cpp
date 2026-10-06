//=============================================================================
//      3D Game Program                        ver 3.2 2023.1.31
//      Main.cpp
//=============================================================================
#define _CRTDBG_MAPALLOC
#include <crtdbg.h>
#include <locale.h>

#include "MainControl.h"
#pragma warning(disable : 28251)

#include "GameMain.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui_node_editor.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg,
                                              WPARAM wParam, LPARAM lParam);

namespace ed = ax::NodeEditor;

// Global Variable
static CMain *g_pMain = nullptr;

//------------------------------------------------------------------------
// Entry Point
//------------------------------------------------------------------------
INT WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, INT) {
  // Memory Leak Check
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

  // CWD を「Data フォルダを含むプロジェクト直下」へ固定する。
  // exe は <ProjectDir>\x64\<Config>\ に出力されるため、exe のフォルダから
  // 上位へ辿り Data フォルダが見つかったディレクトリを基準にする。
  // これにより Data/ や x64/<Config>/*.cso などの相対パスが、
  // 起動方法（F5 / exe 直接実行 等）に依存せず正しく解決される。
  {
    WCHAR exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    WCHAR *lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash) {
      *lastSlash = L'\0'; // exe のあるフォルダ
      WCHAR baseDir[MAX_PATH];
      wcscpy_s(baseDir, exePath);
      for (int level = 0; level < 5; ++level) {
        WCHAR probe[MAX_PATH];
        swprintf_s(probe, L"%s\\Data", baseDir);
        DWORD attr = GetFileAttributesW(probe);
        if (attr != INVALID_FILE_ATTRIBUTES &&
            (attr & FILE_ATTRIBUTE_DIRECTORY)) {
          SetCurrentDirectoryW(baseDir); // Data を含む階層をCWDに設定
          break;
        }
        WCHAR *slash = wcsrchr(baseDir, L'\\');
        if (!slash) {
          break; // これ以上は遡れない（CWDは変更しない）
        }
        *slash = L'\0'; // 一階層上へ
      }
    }
  }

  // Locale
  _tsetlocale(LC_ALL, _T(""));

  CMain *pMain = new CMain;
  g_pMain = pMain;

  bool init_success = false;

  if (SUCCEEDED(pMain->InitWindow(hInstance, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                                  APP_NAME))) {
    if (SUCCEEDED(pMain->Init())) {
      init_success = true;
      pMain->MessageLoop();
    }
  }

  // Cleanup
  pMain->Quit();
  delete pMain;

  return 0;
}

//------------------------------------------------------------------------
// Window Procedure
//------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
    return true;
  }
  if (g_pMain) {
    return g_pMain->MsgProc(hWnd, uMsg, wParam, lParam);
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// ============================================================================================
// CMain
// ============================================================================================
CMain::CMain() {
  ZeroMemory(this, sizeof(CMain));
  m_bLoopFlag = true;
  m_MainLoopTime = 1000000.0 / 60; // 60 FPS
}

CMain::~CMain() { SAFE_DELETE(m_pGMain); }

//------------------------------------------------------------------------
// Init Window
//------------------------------------------------------------------------
HRESULT CMain::InitWindow(HINSTANCE hInstance, INT iX, INT iY, INT iWidth,
                          INT iHeight, LPCTSTR WindowName) {
  m_hInstance = hInstance;

  WNDCLASSEX wc;
  ZeroMemory(&wc, sizeof(wc));
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
  wc.lpszClassName = WindowName;
  wc.lpszMenuName = nullptr;
  RegisterClassEx(&wc);

  RECT rc = {0, 0, iWidth, iHeight};
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

  m_hWnd = CreateWindowEx(0, WindowName, WindowName, WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
                          rc.bottom - rc.top, (HWND) nullptr, (HMENU) nullptr,
                          hInstance, (LPVOID) nullptr);

  if (!m_hWnd) {
    MessageBox(0, _T("Cannot create window"), nullptr, MB_OK);
    return E_FAIL;
  }

  ShowWindow(m_hWnd, SW_SHOW);
  UpdateWindow(m_hWnd);

  return S_OK;
}

//------------------------------------------------------------------------
// MsgProc
//------------------------------------------------------------------------
LRESULT CMain::MsgProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
  switch (iMsg) {
  case WM_KEYDOWN:
    switch ((char)wParam) {
    // ESC key handling removed to allow SceneManager to handle it
    /*
    case VK_ESCAPE:
       DestroyWindow(hWnd);
       PostQuitMessage(0);
       break;
    */
    default:
      break;
    }
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  }

  ImGui_ImplWin32_WndProcHandler(hWnd, iMsg, wParam, lParam);

  return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

//------------------------------------------------------------------------
// MessageLoop
//------------------------------------------------------------------------
void CMain::MessageLoop() {
  // Use VSync mode (Application does not wait)
  MSG msg = {0};
  ZeroMemory(&msg, sizeof(msg));

  while (msg.message != WM_QUIT && m_bLoopFlag) {
    // ImGui Loop is handled in CGameMain::Update/Draw execution flow.
    // Removing redundant calls here.

    m_pGMain->Update();
    m_pGMain->Draw();

    DispFps();

    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}

//------------------------------------------------------------------------
// MessageProcess
//------------------------------------------------------------------------
bool CMain::MessageProcess(MSG *pMsg) {
  while ((pMsg->message != WM_QUIT && m_bLoopFlag) &&
         PeekMessage(pMsg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(pMsg);
    DispatchMessage(pMsg);
  }

  if (pMsg->message == WM_QUIT || m_bLoopFlag == false) {
    return false;
  } else {
    return true;
  }
}

//------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------
static bool m_bImGuiInitialized = false;

HRESULT CMain::Init() {
  m_pGMain = new CGameMain(this);
  m_pGMain->Init();

  return S_OK;
}

//------------------------------------------------------------------------
// Loop
//------------------------------------------------------------------------
void CMain::Loop() {
  m_pGMain->Draw();
  m_pGMain->Update();
  DispFps();
}

//------------------------------------------------------------------------
// Quit
//------------------------------------------------------------------------
void CMain::Quit() {
  m_pGMain->Quit();

  if (m_bImGuiInitialized) {
    ed::DestroyEditor(ed::GetCurrentEditor());
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    m_bImGuiInitialized = false;
  }
}

//------------------------------------------------------------------------
// DispFps
//------------------------------------------------------------------------
void CMain::DispFps() {
  static DWORD time = 0;
  static int frame = 0;
  frame++;
  TCHAR str[50];
  _stprintf_s(str, _T("    fps=%d"), frame);
  if (timeGetTime() - time > 1000) {
    time = timeGetTime();
    frame = 0;
    TCHAR AppName[256] = {0};
    GetClassName(m_hWnd, AppName, sizeof(AppName) / sizeof(TCHAR));
    _tcscat_s(AppName, str);
    SetWindowText(m_hWnd, AppName);
  }
}
