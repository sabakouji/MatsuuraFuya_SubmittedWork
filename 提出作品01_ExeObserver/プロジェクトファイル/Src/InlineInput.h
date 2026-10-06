#pragma once
#include "CoreData.h"
#include "GameObject.h"
#include <functional>
#include <string>

enum class InlineInputType { Text, Float, Int, Bool, Enum, String };

struct InlineInputField {
  std::string id;
  VECTOR2 position = {0.0f, 0.0f};
  float width = 60.0f;
  float height = 18.0f;

  InlineInputType type;
  std::string *valuePtr;

  std::vector<std::string> enumValues;

  float minValue = -10000.0f; // Float/Intの場合の最小値
  float maxValue = 10000.0f;  // Float/Intの場合の最大値
  float dragSpeed = 0.1f;     // Float/Intの場合のドラッグ速度

  bool isHovered = false;
};

class InlineInputManager {
public:
  InlineInputManager();
  ~InlineInputManager() = default;

  void BeginFrame();
  void RegisterField(const InlineInputField &field);
  void EndFrame();

  void Draw(float zoomScale = 1.0f);

  bool ProcessInput(const VECTOR2 &mousePos, bool isLeftClickDown,
                    bool isLeftClickUp);

  bool HasActiveField() const { return !activeFieldId_.empty(); }

  bool WasValueChanged() const { return valueChanged_; }

  void ProcessKeyboardInput();

  bool IsValueChanged() const { return valueChanged_; }
  bool WasEditFinished() const { return editFinished_; }

private:
  std::vector<InlineInputField> fields_;
  std::string activeFieldId_;
  std::string editBuffer_;
  int cursorPos_;
  float cursorBlinkTimer_;
  bool cursorVisible_;
  bool valueChanged_;
  bool editFinished_;
  bool wasLeftClickDown_;

  bool isDragging_;
  VECTOR2 dragStartPos_;
  float dragStartValue_;

  void DrawTextField(const InlineInputField &field, bool isActive);
  void DrawBoolField(const InlineInputField &field);
  void DrawEnumField(const InlineInputField &field, bool isActive);

  InlineInputField *HitTest(const VECTOR2 &pos);

  void ApplyValue();
  void CancelEdit();

  bool showEnumDropdown_;
  std::string enumDropdownFieldId_;
  VECTOR2 enumDropdownPos_;
  int enumHoverIndex_;
};