#pragma once
#include "../Libs/Imgui/imgui.h"
#include "../Libs/Imgui/imgui_node_editor.h"
#include "GameObject.h"
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace ed = ax::NodeEditor;

//=============================================================================
// ターゲットボックスタイプ（共有定義）
//=============================================================================

enum class TargetBoxType {
  None,    // ボックスを使わない（前方移動）
  All,     // 全オブジェクト
  Enemy,   // エネミー
  Door,    // ドア
  Item,    // アイテム
  Goal,    // ゴール
  Position // 指定座標
};

//=============================================================================
// ピンタイプ
//=============================================================================

enum class PinType {
  Control,
  ControlIn,
  ControlOut,
  DataIn,
  DataOut,
  DataIn_Int,
  DataOut_Int,
  DataIn_Float,
  DataOut_Float,
  DataIn_String,
  DataOut_String,
  DataIn_Bool,
  DataOut_Bool,
  DataIn_Enum,
  DataOut_Enum,
  DataIn_Vector3,
  DataOut_Vector3,
  Unknown,
  DataIn_Generic,
  DataOut_Generic,
  DataIn_Skill,
  DataOut_Skill
};

//=============================================================================
// プロパティ型（エディタ用）
//=============================================================================

enum class PropertyType { Float, Int, Bool, String, Enum, Vector3 };

//=============================================================================
// Enum定義（プロパティで使用するenum一覧）
//=============================================================================

struct EnumDefinition {
  std::string name;
  std::vector<std::string> values;

  int GetIndex(const std::string &value) const {
    for (size_t i = 0; i < values.size(); i++) {
      if (values[i] == value)
        return static_cast<int>(i);
    }
    return 0;
  }

  const std::string &GetValue(int index) const {
    if (index < 0 || index >= static_cast<int>(values.size())) {
      return values[index];
    }
    static std::string empty = "";
    return empty;
  }
};

//=============================================================================
// プロパティ定義（テンプレート用）
//=============================================================================

struct PropertyDefinition {
  std::string name;
  PropertyType type;
  std::string defaultValue;
  std::string enumTypeName; // Enumの場合、対応するEnumDefinitionの名前
  float minValue = 0.0f;    // Float/Intの場合の最小値
  float maxValue = 0.0f;    // Float/Intの場合の最大値
  bool hasDataPin = false;  // データピンが存在するか
  bool isVisible = true;    // プロパティパネルに表示するか
};

//=============================================================================
// ビヘイビアステート
//=============================================================================

enum class BehaviorState { SUCCESS, FAILURE, RUNNING, READY };

//=============================================================================
// ピンインスタンス
//=============================================================================

struct PinInstance {
  std::string id;
  std::string name;
  PinType type;
  std::string parentNodeId;
  VECTOR2 position;
  std::vector<std::string> linkedPinIds;
  std::string dataValue;
  std::string linkedPropertyName;

  bool IsInput() const {
    return type == PinType::ControlIn || type == PinType::DataIn ||
           type == PinType::DataIn_Int || type == PinType::DataIn_Float ||
           type == PinType::DataIn_String || type == PinType::DataIn_Vector3 ||
           type == PinType::DataIn_Generic || type == PinType::DataIn_Skill ||
           type == PinType::DataIn_Bool || type == PinType::DataIn_Enum;
  }

  bool IsDataPin() const {
    return type != PinType::Control && type != PinType::ControlIn &&
           type != PinType::ControlOut;
  }

  bool IsConnected() const { return !linkedPinIds.empty(); }
};

//=============================================================================
// ノードインスタンス
//=============================================================================

struct NodeInstance {
  std::string id;
  std::string typeId;
  std::string displayName;
  std::vector<PinInstance> inputPins;
  std::vector<PinInstance> outputPins;
  std::map<std::string, std::string> properties;
  VECTOR2 position;
  std::vector<std::string> linkedPinIds;

  std::map<std::string, PropertyDefinition> propertyDefinitions;

  BehaviorState currentState = BehaviorState::READY;

  bool isSelected = false;
  bool isHovered = false;

  bool operator<(const NodeInstance &other) const { return id < other.id; }

  void Draw(const VECTOR2 &cameraOffset, float scale = 1.0f) const;
  void DrawPreview(const VECTOR2 &position, float scale = 1.0f) const;

  bool DrawInlineInputs(const VECTOR2 &cameraOffset);
  bool DrawPropertyEditor(const VECTOR2 &screenPos);

  bool HitTest(const VECTOR2 &point) const;
  PinInstance *HitTestPin(const VECTOR2 &point);

  float GetWidth() const;
  float GetHeight() const;

  std::string GetPropertyValue(const std::string &propName) const;
  PinInstance *GetPropertyDataPin(const std::string &propName);
  const PinInstance *GetPropertyDataPin(const std::string &propName) const;
};

//=============================================================================
// リンクインスタンス
//=============================================================================

struct LinkInstance {
  std::string id;
  std::string startPinId;
  std::string endPinId;

  // 接続の整合性チェック
  bool IsValid(const std::map<std::string, PinInstance> &allPins) const;
};

//=============================================================================
// グラフデータ
//=============================================================================

struct GraphData {
  std::vector<NodeInstance> nodes;
  std::vector<LinkInstance> links;
  long long nextNodeId = 1;
  long long nextLinkId = 1;

  std::map<std::string, PinInstance> allPins;
};

//=============================================================================
// Enum定義のグローバルレジストリ
//=============================================================================

class EnumRegistry {
public:
  static EnumRegistry &GetInstance() {
    static EnumRegistry instance;
    return instance;
  }
  void RegisterEnum(const std::string &name,
                    const std::vector<std::string> &values) {
    EnumDefinition def;
    def.name = name;
    def.values = values;
    enums_[name] = def;
  }
  const EnumDefinition *GetEnum(const std::string &name) const {
    auto it = enums_.find(name);
    if (it != enums_.end()) {
      return &it->second;
    }
    return nullptr;
  }

private:
  EnumRegistry() {
    RegisterEnum("TargetBoxType",
                 {"None", "All", "Enemy", "Door", "Item", "Goal", "Position"});
    RegisterEnum("Boolean", {"false", "true"});
  }
  std::map<std::string, EnumDefinition> enums_;
};