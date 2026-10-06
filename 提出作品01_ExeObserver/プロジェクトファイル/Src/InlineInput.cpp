// 44行目 [ProcessInput] Handle input for fields
// 518行目 [ApplyValue] Apply edited value

#include "InlineInput.h"
#include "CDrawUtil.h"
#include <algorithm>
#include <cmath>

namespace InputColors {
constexpr int BG_NORMAL = 0xFF2A2A2A;
constexpr int BG_HOVER = 0xFF3A3A3A;
constexpr int BG_ACTIVE = 0xFF1A1A1A;
constexpr int BORDER_NORMAL = 0xFF555555;
constexpr int BORDER_ACTIVE = 0xFF00BFFF;
constexpr int TEXT = 0xFFFFFFFF;
constexpr int CURSOR = 0xFFFFFFFF;
constexpr int CHECKBOX_CHECK = 0xFF00BFFF;
constexpr int DROPDOWN_BG = 0xFF2A2A2A;
constexpr int DROPDOWN_HOVER = 0xFF00BFFF;
} // namespace InputColors

//-----------------------------------------------------------------------------
// コンストラクタ
InlineInputManager::InlineInputManager() {
  cursorPos_ = 0;
  cursorBlinkTimer_ = 0.0f;
  cursorVisible_ = true;
  valueChanged_ = false;
  wasLeftClickDown_ = false;
  isDragging_ = false;
  dragStartValue_ = 0.0f;
  showEnumDropdown_ = false;
  enumHoverIndex_ = -1;
  editFinished_ = false;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// フレーム開始時の初期化
void InlineInputManager::BeginFrame() {
  fields_.clear();
  valueChanged_ = false;
  editFinished_ = false;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// インライン入力フィールドの登録
void InlineInputManager::RegisterField(const InlineInputField &field) {
  fields_.push_back(field);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// フレーム終了時の処理
void InlineInputManager::EndFrame() {
  cursorBlinkTimer_ += 1.0f / 60.0f; // 60FPS想定
  if (cursorBlinkTimer_ >= 0.5f) {
    cursorBlinkTimer_ = 0.0f;
    cursorVisible_ = !cursorVisible_;
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// マウス入力の処理
bool InlineInputManager::ProcessInput(const VECTOR2 &mousePos,
                                      bool isLeftClickDown,
                                      bool isLeftClickUp) {
  bool captured = false;
  bool leftPressed = (isLeftClickDown && !wasLeftClickDown_);
  bool leftReleased = isLeftClickUp;
  // ホバー状態の更新
  for (auto &field : fields_) {
    float w = field.width;
    if (field.type == InlineInputType::Enum)
      w += 20.0f;
    field.isHovered =
        (mousePos.x >= field.position.x && mousePos.x <= field.position.x + w &&
         mousePos.y >= field.position.y &&
         mousePos.y <= field.position.y + field.height);
  }
  // Enumドロップダウンが開いている場合の処理
  if (showEnumDropdown_) {
    InlineInputField *enumField = nullptr;
    for (auto &f : fields_) {
      if (f.id == enumDropdownFieldId_) {
        enumField = &f;
        break;
      }
    }
    if (enumField && !enumField->enumValues.empty()) {
      float itemHeight = enumField->height;
      float dropdownWidth = enumField->width + 20.0f;
      float dropdownHeight = enumField->enumValues.size() * itemHeight;
      bool inDropdown = (mousePos.x >= enumDropdownPos_.x &&
                         mousePos.x <= enumDropdownPos_.x + dropdownWidth &&
                         mousePos.y >= enumDropdownPos_.y &&
                         mousePos.y <= enumDropdownPos_.y + dropdownHeight);
      if (inDropdown) {
        enumHoverIndex_ = (int)((mousePos.y - enumDropdownPos_.y) / itemHeight);
        if (leftPressed && enumHoverIndex_ >= 0 &&
            enumHoverIndex_ < (int)enumField->enumValues.size()) {
          if (enumField->valuePtr) {
            *enumField->valuePtr = enumField->enumValues[enumHoverIndex_];
            valueChanged_ = true;
            editFinished_ = true;
          }
          showEnumDropdown_ = false;
        }
        captured = true;
      } else if (leftPressed) {
        showEnumDropdown_ = false;
      }
    }
    wasLeftClickDown_ = isLeftClickDown;
    return captured || showEnumDropdown_;
  }
  // フィールドクリック処理
  if (leftPressed) {
    InlineInputField *hitField = HitTest(mousePos);
    if (hitField) {
      // Bool型は即座にトグル
      if (hitField->type == InlineInputType::Bool) {
        if (hitField->valuePtr) {
          bool currentVal =
              (*hitField->valuePtr == "true" || *hitField->valuePtr == "1");
          *hitField->valuePtr = currentVal ? "false" : "true";
          valueChanged_ = true;
          editFinished_ = true;
        }
        captured = true;
      }
      // Enum型はドロップダウンを開く
      else if (hitField->type == InlineInputType::Enum) {
        showEnumDropdown_ = true;
        enumDropdownFieldId_ = hitField->id;
        // フィールドの直下に表示
        enumDropdownPos_.x = hitField->position.x;
        enumDropdownPos_.y = hitField->position.y + hitField->height;
        enumHoverIndex_ = -1;
        captured = true;
      }
      // その他の型は編集モードに入る
      else {
        if (activeFieldId_ != hitField->id) {
          ApplyValue();
          activeFieldId_ = hitField->id;
          editBuffer_ = hitField->valuePtr ? *hitField->valuePtr : "";
          cursorPos_ = (int)editBuffer_.length();
          cursorBlinkTimer_ = 0.0f;
          cursorVisible_ = true;
        }
        // Float/Int型はドラッグ開始
        if (hitField->type == InlineInputType::Float ||
            hitField->type == InlineInputType::Int) {
          isDragging_ = true;
          dragStartPos_ = mousePos;
          try {
            dragStartValue_ = std::stof(editBuffer_);
          } catch (...) {
            dragStartValue_ = 0.0f;
          }
        }
        captured = true;
      }
    } else {
      ApplyValue();
      activeFieldId_.clear();
    }
  }
  // ドラッグ中の処理
  if (isDragging_ && isLeftClickDown) {
    InlineInputField *activeField = nullptr;
    for (auto &f : fields_) {
      if (f.id == activeFieldId_) {
        activeField = &f;
        break;
      }
    }
    if (activeField) {
      float delta = (mousePos.x - dragStartPos_.x) * activeField->dragSpeed;
      float newValue = dragStartValue_ + delta;
      newValue =
          std::max<float>(activeField->minValue,
                          std::min<float>(activeField->maxValue, newValue));
      if (activeField->type == InlineInputType::Int) {
        editBuffer_ = std::to_string((int)newValue);
      } else {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.3f", newValue);
        editBuffer_ = buf;
      }
      if (activeField->valuePtr) {
        *activeField->valuePtr = editBuffer_;
        valueChanged_ = true;
      }
    }
    captured = true;
  }
  if (leftReleased) {
    if (isDragging_)
      editFinished_ = true;
    isDragging_ = false;
  }
  wasLeftClickDown_ = isLeftClickDown;
  if (!activeFieldId_.empty()) {
    captured = true;
  }
  return captured;
}
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// キーボード入力の処理
void InlineInputManager::ProcessKeyboardInput() {
  if (activeFieldId_.empty())
    return;
  InlineInputField *activeField = nullptr;
  for (auto &f : fields_) {
    if (f.id == activeFieldId_) {
      activeField = &f;
      break;
    }
  }
  if (!activeField)
    return;
  // Escapeで編集キャンセル
  if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_ESCAPE)) {
    CancelEdit();
    return;
  }
  // Enterで編集確定
  if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_RETURN) ||
      GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_NUMPADENTER)) {
    ApplyValue();
    activeFieldId_.clear();
    return;
  }
  // Backspace
  if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_BACK) ||
      GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_BACK)) {
    static float backspaceTimer = 0.0f;
    bool doBackspace = GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_BACK);
    if (GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_BACK)) {
      backspaceTimer += 1.0f / 60.0f;
      if (backspaceTimer > 0.5f) {
        doBackspace = true;
        backspaceTimer = 0.45f; // リピート間隔
      }
    } else {
      backspaceTimer = 0.0f;
    }
    if (doBackspace && cursorPos_ > 0) {
      editBuffer_.erase(cursorPos_ - 1, 1);
      cursorPos_--;
      cursorBlinkTimer_ = 0.0f;
      cursorVisible_ = true;
    }
  }
  // Delete
  if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_DELETE)) {
    if (cursorPos_ < (int)editBuffer_.length()) {
      editBuffer_.erase(cursorPos_, 1);
    }
  }
  // カーソル移動
  if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_LEFT)) {
    if (cursorPos_ > 0)
      cursorPos_--;
    cursorBlinkTimer_ = 0.0f;
    cursorVisible_ = true;
  }
  if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_RIGHT)) {
    if (cursorPos_ < (int)editBuffer_.length())
      cursorPos_++;
    cursorBlinkTimer_ = 0.0f;
    cursorVisible_ = true;
  }
  if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_HOME)) {
    cursorPos_ = 0;
    cursorBlinkTimer_ = 0.0f;
    cursorVisible_ = true;
  }
  if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_END)) {
    cursorPos_ = (int)editBuffer_.length();
    cursorBlinkTimer_ = 0.0f;
    cursorVisible_ = true;
  }
  // 文字入力（数字、マイナス、ピリオド）
  struct KeyMap {
    int dik;
    char ch;
    char shiftCh;
  };
  static const KeyMap keyMap[] = {
      {DIK_0, '0', ')'},       {DIK_1, '1', '!'},
      {DIK_2, '2', '@'},       {DIK_3, '3', '#'},
      {DIK_4, '4', '$'},       {DIK_5, '5', '%'},
      {DIK_6, '6', '^'},       {DIK_7, '7', '&'},
      {DIK_8, '8', '*'},       {DIK_9, '9', '('},
      {DIK_NUMPAD0, '0', '0'}, {DIK_NUMPAD1, '1', '1'},
      {DIK_NUMPAD2, '2', '2'}, {DIK_NUMPAD3, '3', '3'},
      {DIK_NUMPAD4, '4', '4'}, {DIK_NUMPAD5, '5', '5'},
      {DIK_NUMPAD6, '6', '6'}, {DIK_NUMPAD7, '7', '7'},
      {DIK_NUMPAD8, '8', '8'}, {DIK_NUMPAD9, '9', '9'},
      {DIK_MINUS, '-', '_'},   {DIK_NUMPADMINUS, '-', '-'},
      {DIK_PERIOD, '.', '>'},  {DIK_NUMPADPERIOD, '.', '.'},
  };
  bool shiftDown = GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_LSHIFT) ||
                   GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_RSHIFT);
  for (const auto &km : keyMap) {
    if (GameDevice()->m_pDI->CheckKey(KD_TRG, km.dik)) {
      char ch = shiftDown ? km.shiftCh : km.ch;
      if (activeField->type == InlineInputType::Int ||
          activeField->type == InlineInputType::Float) {
        bool valid = (ch >= '0' && ch <= '9');
        if (ch == '-' && cursorPos_ == 0 &&
            editBuffer_.find('-') == std::string::npos) {
          valid = true;
        }
        if (ch == '.' && activeField->type == InlineInputType::Float &&
            editBuffer_.find('.') == std::string::npos) {
          valid = true;
        }
        if (!valid)
          continue;
      }
      editBuffer_.insert(cursorPos_, 1, ch);
      cursorPos_++;
      cursorBlinkTimer_ = 0.0f;
      cursorVisible_ = true;
    }
  }
  if (activeField->type == InlineInputType::Text) {
    for (int i = 0; i < 26; ++i) {
      if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_A + i)) {
        char ch = shiftDown ? ('A' + i) : ('a' + i);
        editBuffer_.insert(cursorPos_, 1, ch);
        cursorPos_++;
        cursorBlinkTimer_ = 0.0f;
        cursorVisible_ = true;
      }
    }
    if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_SPACE)) {
      editBuffer_.insert(cursorPos_, 1, ' ');
      cursorPos_++;
    }
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// インライン入力の描画
void InlineInputManager::Draw(float zoomScale) {
  if (fields_.empty())
    return;
  for (const auto &field : fields_) {
    if (field.width <= 0.0f || field.height <= 0.0f)
      continue;
    if (field.valuePtr == nullptr)
      continue;
    bool isActive = (field.id == activeFieldId_);
    switch (field.type) {
    case InlineInputType::Bool:
      DrawBoolField(field);
      break;
    case InlineInputType::Enum:
      DrawEnumField(field, isActive);
      break;
    case InlineInputType::Float:
    case InlineInputType::Int:
    case InlineInputType::Text:
    default:
      DrawTextField(field, isActive);
      break;
    }
  }
  if (showEnumDropdown_) {
    InlineInputField *enumField = nullptr;
    for (auto &f : fields_) {
      if (f.id == enumDropdownFieldId_) {
        enumField = &f;
        break;
      }
    }
    if (enumField && !enumField->enumValues.empty()) {
      enumDropdownPos_.x = enumField->position.x;
      enumDropdownPos_.y = enumField->position.y + enumField->height;
      float itemHeight = enumField->height;
      float dropdownwidth = enumField->width + 20.0f;
      float dropdownHeight = enumField->enumValues.size() * itemHeight;
      CDrawUtil::DrawRect(enumDropdownPos_.x, enumDropdownPos_.y, dropdownwidth,
                          dropdownHeight, InputColors::DROPDOWN_BG);
      for (size_t i = 0; i < enumField->enumValues.size(); ++i) {
        float itemY = enumDropdownPos_.y + i * itemHeight;
        if ((int)i == enumHoverIndex_) {
          CDrawUtil::DrawRect(enumDropdownPos_.x, itemY, dropdownwidth,
                              itemHeight, InputColors::DROPDOWN_HOVER);
        }
        CDrawUtil::DrawText(
            enumField->enumValues[i].c_str(), enumDropdownPos_.x + 4.0f,
            itemY + 2.0f * zoomScale,
            InputColors::TEXT); // Adjusted offset for zoom? - Text scaling not
                                // typically supported by CDrawUtil directly
                                // unless modified or using ImGui scaling.
                                // Assuming standard font size, position
                                // adjustment might be needed.
        // Wait, the request is "Imgui text should scale down". CDrawUtil wraps
        // ImGui/D3D text? CDrawUtil::DrawText uses
        // ImGui::GetBackgroundDrawList()->AddText? Or D3DXFont? If it uses
        // ImGui, we can scale font size. Let's assume for now we just pass
        // scale if CDrawUtil supported it, but it likely doesn't. However, the
        // USER request said "Imgui text". If CDrawUtil::DrawText uses ImGui, we
        // might need to set font scale. Checking CDrawUtil implementation would
        // be ideal but I can't see it right now. For now, I'll assume I can't
        // easily scale standard DrawText. BUT, the fields themselves position
        // and resize based on zoom logic in NodeGraph code? "Inline input
        // fields" are managed here. NodeGraph calls RegisterInlineInputFields
        // which calculates position and size based on Zoom. So the rects are
        // already scaled. The TEXT inside them needs to scale. Standard ImGui
        // text doesn't scale unless we FontGlobalScale.
      }
    }
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// テキストフィールドの描画
void InlineInputManager::DrawTextField(const InlineInputField &field,
                                       bool isActive) {
  int bgColor = InputColors::BG_NORMAL;
  int borderColor = InputColors::BORDER_NORMAL;
  if (isActive) {
    bgColor = InputColors::BG_ACTIVE;
    borderColor = InputColors::BORDER_ACTIVE;
  } else if (field.isHovered) {
    bgColor = InputColors::BG_HOVER;
  }
  CDrawUtil::DrawRect(field.position.x, field.position.y, field.width,
                      field.height, bgColor);
  ImDrawList *dl = ImGui::GetBackgroundDrawList();
  dl->AddRect(
      ImVec2(field.position.x, field.position.y),
      ImVec2(field.position.x + field.width, field.position.y + field.height),
      isActive ? IM_COL32(0, 191, 255, 255) : IM_COL32(85, 85, 85, 255), 2.0f,
      0, 1.0f);
  const std::string &displayText =
      isActive ? editBuffer_ : (field.valuePtr ? *field.valuePtr : "");
  float textX = field.position.x + 4.0f;
  float textY = field.position.y +
                (field.height - 14.0f) /
                    2.0f; // This centers vertically based on assumed 14px font?

  // Calculate scale based on field height (which is scaled by zoom in
  // NodeGraph) Base height seems to be 24.0f
  float fontScale = field.height / 24.0f;
  if (fontScale < 0.1f)
    fontScale = 0.1f;

  // Use safe font scaling by passing font and size to AddText directly
  ImFont *font = ImGui::GetFont();
  float fontSize = ImGui::GetFontSize() * fontScale;

  dl->AddText(font, fontSize, ImVec2(textX, textY),
              IM_COL32(255, 255, 255, 255), displayText.c_str());

  if (isActive && cursorVisible_ && !isDragging_) {
    // Calculate text size with the scaled font
    ImVec2 textSize = font->CalcTextSizeA(
        fontSize, FLT_MAX, 0.0f, displayText.substr(0, cursorPos_).c_str());

    float cursorX = textX + textSize.x;
    dl->AddLine(ImVec2(cursorX, field.position.y + 2.0f),
                ImVec2(cursorX, field.position.y + field.height - 2.0f),
                IM_COL32(255, 255, 255, 255), 1.0f);
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ブール値フィールドの描画（チェックボックス）
void InlineInputManager::DrawBoolField(const InlineInputField &field) {
  bool value =
      field.valuePtr && (*field.valuePtr == "true" || *field.valuePtr == "1");
  float boxSize = field.height - 4.0f;
  float boxX = field.position.x + 2.0f;
  float boxY = field.position.y + 2.0f;
  int bgColor =
      field.isHovered ? InputColors::BG_HOVER : InputColors::BG_NORMAL;
  CDrawUtil::DrawRect(boxX, boxY, boxSize, boxSize, bgColor);
  ImDrawList *dl = ImGui::GetBackgroundDrawList();
  dl->AddRect(ImVec2(boxX, boxY), ImVec2(boxX + boxSize, boxY + boxSize),
              IM_COL32(85, 85, 85, 255), 2.0f, 0, 1.0f);
  if (value) {
    float cx = boxX + boxSize / 2.0f;
    float cy = boxY + boxSize / 2.0f;
    float r = boxSize / 2.0f - 3.0f;
    dl->AddLine(ImVec2(cx - r * 0.5f, cy), ImVec2(cx, cy + r),
                IM_COL32(0, 191, 255, 255), 2.0f);
    dl->AddLine(ImVec2(cx - r * 0.1f, cy + r * 0.5f),
                ImVec2(cx + r * 0.6f, cy - r * 0.4f),
                IM_COL32(0, 191, 255, 255), 2.0f);
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 列挙型フィールドの描画（ドロップダウン）
void InlineInputManager::DrawEnumField(const InlineInputField &field,
                                       bool isActive) {
  int bgColor = InputColors::BG_NORMAL;
  if (isActive) {
    bgColor = InputColors::BG_ACTIVE;
  } else if (field.isHovered) {
    bgColor = InputColors::BG_HOVER;
  }
  CDrawUtil::DrawRect(field.position.x, field.position.y, field.width + 20.0f,
                      field.height, bgColor);
  ImDrawList *dl = ImGui::GetBackgroundDrawList();
  bool highlighted =
      isActive || (showEnumDropdown_ && enumDropdownFieldId_ == field.id);
  dl->AddRect(ImVec2(field.position.x, field.position.y),
              ImVec2(field.position.x + field.width + 20.0f,
                     field.position.y + field.height),
              highlighted ? IM_COL32(0, 191, 255, 255)
                          : IM_COL32(85, 85, 85, 255),
              2.0f, 0, 1.0f);
  const std::string &displayText = field.valuePtr ? *field.valuePtr : "";
  float textY = field.position.y + (field.height - 14.0f) / 2.0f;
  // Scale text
  float fontScale = field.height / 24.0f; // Derive scale from height
  if (fontScale < 0.1f)
    fontScale = 0.1f;
  dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * fontScale,
              ImVec2(field.position.x + 4.0f, textY),
              IM_COL32(255, 255, 255, 255), displayText.c_str());
  // CDrawUtil::DrawText(displayText.c_str(), field.position.x + 4.0f, textY,
  // InputColors::TEXT);
  float arrowX = field.position.x + field.width + 10.0f;
  float arrowY = field.position.y + field.height / 2.0f;
  dl->AddTriangleFilled(
      ImVec2(arrowX - 4.0f * fontScale, arrowY - 2.0f * fontScale),
      ImVec2(arrowX + 4.0f * fontScale, arrowY - 2.0f * fontScale),
      ImVec2(arrowX, arrowY + 4.0f * fontScale), IM_COL32(255, 255, 255, 200));
}
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ヒットテスト（マウス位置にあるフィールドを取得）
InlineInputField *InlineInputManager::HitTest(const VECTOR2 &Pos) {
  for (auto &field : fields_) {
    float w = field.width;
    if (field.type == InlineInputType::Enum) {
      w += 20.0f;
    }
    if (Pos.x >= field.position.x && Pos.x <= field.position.x + w &&
        Pos.y >= field.position.y && Pos.y <= field.position.y + field.height) {
      return &field;
    }
  }
  return nullptr;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 編集内容の確定と適用
void InlineInputManager::ApplyValue() {
  if (activeFieldId_.empty())
    return;
  for (auto &f : fields_) {
    if (f.id == activeFieldId_ && f.valuePtr) {
      if (f.type == InlineInputType::Float) {
        try {
          float val = std::stof(editBuffer_);
          val = std::max<float>(f.minValue, std::min<float>(f.maxValue, val));
          char buf[32];
          snprintf(buf, sizeof(buf), "%.3f", val);
          editBuffer_ = buf;
        } catch (...) {
          editBuffer_ = "0.000";
        }
      } else if (f.type == InlineInputType::Int) {
        try {
          int val = std::stoi(editBuffer_);
          val = std::max<int>((int)f.minValue,
                              std::min<int>((int)f.maxValue, val));
          editBuffer_ = std::to_string(val);
        } catch (...) {
          editBuffer_ = "0";
        }
      }
      *f.valuePtr = editBuffer_;
      valueChanged_ = true;
      editFinished_ = true;
      break;
    }
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 編集のキャンセル
void InlineInputManager::CancelEdit() {
  activeFieldId_.clear();
  editBuffer_.clear();
  cursorPos_ = 0;
}
//------------------------------------------------------------------------------