//=============================================================================
//
//	MyImgui.cpp
//
//=============================================================================
#include "MyImgui.h"

// convert_utf8
#include <Windows.h>
#include <codecvt>
#include <stdexcept>

#include "imgui_node_editor.h"

namespace ed = ax::NodeEditor;

// File-scope variables (internal linkage ideally, or global if used across
// translation units? Header doesn't declare them extern) In original, they were
// outside namespace MyImgui.
ed::EditorContext *m_pEditorContext = nullptr;
static bool s_ImguiInitialized = false;

//=============================================================================
// MyImgui::ImguiInit
//=============================================================================
void MyImgui::ImguiInit(HWND hWnd, CDirect3D *pD3D, int WidthIn, int HeightIn) {
  if (s_ImguiInitialized)
    return;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  m_pEditorContext = ed::CreateEditor();

  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsClassic();

  if (!ImGui_ImplWin32_Init(hWnd)) {
    MessageBox(0, _T("imguiを初期化出来ません"), nullptr, MB_OK);
  }
  if (!ImGui_ImplDX11_Init(pD3D->m_pDevice.Get(), pD3D->m_pDeviceContext.Get())) {
    MessageBox(0, _T("imguiを初期化出来ません"), nullptr, MB_OK);
  }

  s_ImguiInitialized = true;
  io.IniFilename = nullptr;

  // Font Setup
  ImFontConfig config;
  config.MergeMode = false;

  // 優先順位: HGPゴシックE -> メイリオ -> MSゴシック
  const char *fontPaths[] = {"\\hgrge.ttc", "\\meiryo.ttc", "\\msgothic.ttc"};

  bool fontLoaded = false;
  char FontPath[MAX_PATH];

  for (const char *fontName : fontPaths) {
    if (SHGetSpecialFolderPathA(nullptr, FontPath, CSIDL_FONTS, 0)) {
      strcat_s(FontPath, fontName);
      // ファイルが存在するか簡易チェック（fopenなど）してもよいが、AddFontFromFileTTFが失敗しても次の処理に進むだけなのでそのまま試行
      ImFont *font = io.Fonts->AddFontFromFileTTF(
          FontPath, 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
      if (font) {
        fontLoaded = true;
        break;
      }
    }
  }

  if (!fontLoaded) {
    // フォールバック：デフォルトフォントに日本語レンジを追加（表示できない可能性高いがエラーは防ぐ）
    io.Fonts->AddFontDefault();
  }
}

//=============================================================================
// MyImgui::ImguiQuit
//=============================================================================
void MyImgui::ImguiQuit() {
  if (!s_ImguiInitialized)
    return;

  if (m_pEditorContext) {
    ed::DestroyEditor(m_pEditorContext);
    m_pEditorContext = nullptr;
  }
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  s_ImguiInitialized = false;
}

//=============================================================================
// MyImgui::ImguiNewFrame
//=============================================================================
void MyImgui::ImguiNewFrame() {
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
}

//=============================================================================
// MyImgui::ImguiRender
//=============================================================================
void MyImgui::ImguiRender() {
  // Update(); // Debug logic removed/commented
  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

//=============================================================================
// MyImgui::Update
//=============================================================================
void MyImgui::Update() {
  // Empty implementation to satisfy linker and avoid errors
}

//=============================================================================
// MyImgui::ShowDemoWindow2
//=============================================================================
void MyImgui::ShowDemoWindow2(bool *p_open) {
  if (ImGui::Begin("Demo Window 2", p_open)) {
    ImGui::Text("Demo Window 2 Content Removed for Stability");
  }
  ImGui::End();
}

//=============================================================================
// MyImgui::ConvertUTF8ToTCHAR
//=============================================================================
void MyImgui::ConvertUTF8ToTCHAR(char *charIn, TCHAR *tcharOut) {
#ifdef _UNICODE
  ConvertU8ToU16(charIn, tcharOut);
#else
  wchar_t wstr[512];
  ConvertU8ToU16(charIn, wstr);
  char mstr[512];
  WideCharToMultiByte(CP_ACP, 0, wstr, -1, mstr, 512, nullptr, nullptr);
  strcpy_s(tcharOut, strlen(mstr) + 1, mstr);
#endif
}

//=============================================================================
// MyImgui::ConvertTCHARToUTF8
//=============================================================================
void MyImgui::ConvertTCHARToUTF8(TCHAR *tcharIn, char *charOut) {
#ifdef _UNICODE
  ConvertU16ToU8(tcharIn, charOut);
#else
  wchar_t wstr[512] = {L'\0'};
  MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, tcharIn, -1, wstr, 512);
  ConvertU16ToU8(wstr, charOut);
#endif
}

//=============================================================================
// MyImgui::ConvertU8ToU16
//=============================================================================
void MyImgui::ConvertU8ToU16(char *charIn, WCHAR *wcharOut) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convt;
  std::wstring wch = convt.from_bytes(charIn);
  wcscpy_s(wcharOut, wch.length() + 1, (WCHAR *)wch.c_str());
}

//=============================================================================
// MyImgui::ConvertU16ToU8
//=============================================================================
void MyImgui::ConvertU16ToU8(WCHAR *wcharIn, char *charOut) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convt;
  std::string ch = convt.to_bytes(wcharIn);
  strcpy_s(charOut, ch.length() + 1, ch.c_str());
}

//=============================================================================
// MyImgui::SJIStoUTF8
//=============================================================================
std::string MyImgui::SJIStoUTF8(const std::string &sjis) {
  if (sjis.empty())
    return "";
  int size_needed = MultiByteToWideChar(CP_THREAD_ACP, 0, sjis.c_str(),
                                        (int)sjis.size(), NULL, 0);
  if (size_needed == 0)
    return "";
  std::wstring wstr(size_needed, 0);
  MultiByteToWideChar(CP_THREAD_ACP, 0, sjis.c_str(), (int)sjis.size(),
                      &wstr[0], size_needed);
  int size_needed_utf8 = WideCharToMultiByte(
      CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
  if (size_needed_utf8 == 0)
    return "";
  std::string utf8(size_needed_utf8, 0);
  WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &utf8[0],
                      size_needed_utf8, NULL, NULL);
  return utf8;
}

//=============================================================================
// MyImgui::UTF8toSJIS
//=============================================================================
std::string MyImgui::UTF8toSJIS(const std::string &utf8) {
  if (utf8.empty())
    return "";
  int size_needed =
      MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
  if (size_needed == 0)
    return "";
  std::wstring wstr(size_needed, 0);
  MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wstr[0],
                      size_needed);
  int size_needed_sjis = WideCharToMultiByte(
      CP_THREAD_ACP, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
  if (size_needed_sjis == 0)
    return "";
  std::string sjis(size_needed_sjis, 0);
  WideCharToMultiByte(CP_THREAD_ACP, 0, &wstr[0], (int)wstr.size(), &sjis[0],
                      size_needed_sjis, NULL, NULL);
  return sjis;
}

//=============================================================================
// MyImgui::GetLocalizedText
//=============================================================================
#include <map>
std::string MyImgui::GetLocalizedText(const std::string &key) {
  static const std::map<std::string, std::string> translations = {
      // --- Nodes ---
      {"Root", "開始"},
      {"Sequence", "シーケンス"},
      {"Selector", "セレクター"},
      {"Inverter", "反転"},
      {"Action_Run", "移動(走行)"},
      {"Action_Stop", "停止"},
      {"Vaiable_AttackData", "攻撃データ"},
      {"Condition_IsTargetInAttackRange", "攻撃範囲判定"},
      {"Action_ExecuteAttack", "攻撃実行"},
      {"Data_AttackSkill", "攻撃スキル"},
      {"Action_RadarScan", "レーダースキャン"},
      {"Action_RadarPulse", "レーダーパルス"},
      {"Action_SetTargetPosition", "目標座標設定"},
      {"Action_ClearTargetBox", "対象ボックスクリア"},
      {"Condition_HasTargetsInBox", "対象あり?"},
      {"Condition_IsNearTarget", "近接判定?"},
      {"Condition_InTheEyes", "視野内判定?"},
      {"Condition_If", "条件分岐"},
      {"Variable_Float", "浮動小数"},
      {"Variable_Int", "整数"},
      {"Variable_Bool", "真偽値"},
      {"Variable_TargetBoxType", "対象ボックス型"},
      {"Data_CheckBox", "ボックス確認"},
      {"Action_GetStageTask", "ステージタスク取得"},
      {"Action_FindRouteToGoal", "ゴール経路探索"},
      {"Action_DetectObstacle", "障害物検知"},
      {"Variable_Skill_Fireball", "スキル:火の玉"},
      {"Variable_Skill_SwordSlash", "スキル:斬撃"},
      {"Data_Less", "< (より小さい)"},
      {"Data_Greater", "> (より大きい)"},
      {"Data_LessEqual", "=< (以下)"},
      {"Data_GreaterEqual", "=> (以上)"},
      {"Data_Equal", "== (等しい)"},
      {"Data_NotEqual", "!= (等しくない)"},

      // --- Pins ---
      {"In", "入力"},
      {"Out", "出力"},
      {"Out 1", "出力 1"},
      {"Out 2", "出力 2"},
      {"Out 3", "出力 3"},
      {"Out 4", "出力 4"},
      {"Out 5", "出力 5"},
      {"True", "真"},
      {"False", "偽"},
      {"Condition", "条件"},
      {"Skill", "スキル"},
      {"TargetBox", "対象"},
      {"OperandA", "左辺"},
      {"OperandB", "右辺"},
      {"Result", "結果"},
      {"Speed", "速度"},
      {"ArrivalDist", "到達距離"},
      {"Range", "範囲"},
      {"Damage", "ダメージ"},
      {"Cooldown", "クールダウン"},
      {"MaxRadius", "最大半径"},
      {"ExpandSpeed", "拡大速度"},
      {"Interval", "間隔"},
      {"Radius", "半径"},
      {"X", "X"}, // Keep English/Common
      {"Y", "Y"},
      {"Z", "Z"},
      {"BoxType", "ボックス種別"},
      {"MinTargets", "最小数"},
      {"Distance", "距離"},
      {"Angle", "角度"},
      {"Value", "値"},
      {"SkillID", "スキルID"},
      {"CheckDist", "判定距離"},
      {"CheckRadius", "判定半径"}};

  auto it = translations.find(key);
  if (it != translations.end()) {
    // Translated string is in Shift-JIS (source file encoding), so convert to
    // UTF-8
    return SJIStoUTF8(it->second);
  }

  // Fallback: Convert key (English/ASCII) to UTF-8 (identity usually) just in
  // case
  return SJIStoUTF8(key);
}