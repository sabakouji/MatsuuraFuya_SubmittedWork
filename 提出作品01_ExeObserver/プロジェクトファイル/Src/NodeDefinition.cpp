#include "NodeDefinition.h"
#include "../include/nlohmann/json.hpp"
#include "BehaviorTree_Actions.h"
#include "BehaviorTree_Nodes.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

//-----------------------------------------------------------------------------
// ノード定義のJSONローダー
using json = nlohmann::json;
namespace fs = std::filesystem;
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ピンタイプを文字列に変換
std::string PinTypeToString(PinType type) {
    switch (type) {
    case PinType::ControlIn:
        return "ControlIn";
    case PinType::ControlOut:
        return "ControlOut";
    case PinType::DataIn_Int:
        return "DataIn_Int";
    case PinType::DataOut_Int:
        return "DataOut_Int";
    case PinType::DataIn_Float:
        return "DataIn_Float";
    case PinType::DataOut_Float:
        return "DataOut_Float";
    case PinType::DataIn_String:
        return "DataIn_String";
    case PinType::DataIn_Bool:
        return "DataIn_Bool";
    case PinType::DataOut_Bool:
        return "DataOut_Bool";
    case PinType::DataIn_Enum:
        return "DataIn_Enum";
    case PinType::DataOut_Enum:
        return "DataOut_Enum";
    case PinType::DataOut_String:
        return "DataOut_String";
    case PinType::DataIn_Generic:
        return "DataIn_Generic";
    case PinType::DataOut_Generic:
        return "DataOut_Generic";
    case PinType::DataIn_Skill:
        return "DataIn_Skill";
    case PinType::DataOut_Skill:
        return "DataOut_Skill";
    }
    return "Unknown";
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 文字列をピンタイプに変換
PinType StringToPinType(const std::string& str) {
    if (str == "ControlIn")
        return PinType::ControlIn;
    if (str == "ControlOut")
        return PinType::ControlOut;
    if (str == "DataIn_Int")
        return PinType::DataIn_Int;
    if (str == "DataOut_Int")
        return PinType::DataOut_Int;
    if (str == "DataIn_Float")
        return PinType::DataIn_Float;
    if (str == "DataOut_Float")
        return PinType::DataOut_Float;
    if (str == "DataIn_String")
        return PinType::DataIn_String;
    if (str == "DataOut_String")
        return PinType::DataOut_String;
    if (str == "DataIn_Bool")
        return PinType::DataIn_Bool;
    if (str == "DataOut_Bool")
        return PinType::DataOut_Bool;
    if (str == "DataIn_Enum")
        return PinType::DataIn_Enum;
    if (str == "DataOut_Enum")
        return PinType::DataOut_Enum;
    if (str == "DataIn_Generic")
        return PinType::DataIn_Generic;
    if (str == "DataOut_Generic")
        return PinType::DataOut_Generic;
    if (str == "DataIn_Skill")
        return PinType::DataIn_Skill;
    if (str == "DataOut_Skill")
        return PinType::DataOut_Skill;

    std::cerr << "WARNING: Unknown PinType string: " << str
        << ". Returning ControlIn as fallback." << std::endl;
    return PinType::ControlIn;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードファクトリーの実装
NodeFactory::NodeFactory() { RegisterBuiltinBehaviorNodes(); }
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードファクトリーのシングルトンインスタンス取得
NodeFactory& NodeFactory::GetInstance() {
    static NodeFactory instance;
    return instance;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// JSONファイルからノード定義をロード
bool NodeFactory::LoadDefinitionsFromJson(const std::string& filePath) {
    // ファイルの存在チェック
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) {
        std::cerr << "ERROR: Failed to open node definition file: " << filePath
            << std::endl;
        return false;
    }

    json j;
    try {
        ifs >> j;
    }

    catch (const json::parse_error& e) {
        std::cerr << "ERROR: Failed to parse JSON file: " << filePath << ". "
            << e.what() << std::endl;
        return false;
    }

    size_t loadedCount = 0;

    if (!j.contains("nodes") || !j["nodes"].is_array()) {
        std::cerr << "ERROR: JSON must contain a 'nodes' array." << std::endl;
        return false;
    }

    for (const auto& node_j : j["nodes"]) {
        if (!node_j.contains("name") || !node_j.contains("title") ||
            !node_j.contains("category")) {
            std::cerr << "WARNING: Node definition missing required fields (name, "
                "title, category). Skipping."
                << std::endl;
            continue;
        }

        NodeTemplate tmpl;
        tmpl.typeID = node_j.at("name").get<std::string>();
        tmpl.displayName = node_j.at("title").get<std::string>();
        tmpl.category = node_j.at("category").get<std::string>();

        if (node_j.contains("properties") && node_j["properties"].is_object()) {
            for (auto it = node_j["properties"].begin();
                it != node_j["properties"].end(); ++it) {
                tmpl.defaultProperties[it.key()] = it.value().get<std::string>();
            }
        }

        auto loadPins = [](const json& pins_j,
            std::vector<PinDefinition>& definitions) {
                if (!pins_j.is_array())
                    return;
                for (const auto& pin_j : pins_j) {
                    if (pin_j.contains("name") && pin_j.contains("type")) {
                        PinDefinition def;
                        def.name = pin_j.at("name").get<std::string>();
                        def.type = StringToPinType(pin_j.at("type").get<std::string>());
                        definitions.push_back(def);
                    }
                }
            };

        if (node_j.contains("inputs")) {
            loadPins(node_j["inputs"], tmpl.inputs);
        }

        if (node_j.contains("outputs")) {
            loadPins(node_j["outputs"], tmpl.outputs);
        }

        templates_[tmpl.typeID] = tmpl;
        loadedCount++;
    }

    std::cout << "INFO: Successfully loaded " << loadedCount
        << " node templates from JSON. Total templates: "
        << templates_.size() << std::endl;
    return true;
}

//-----------------------------------------------------------------------------
// ビルトインの振る舞いノードを登録
void NodeFactory::RegisterBuiltinBehaviorNodes() {
    //=========================================================================
    // 基本ノード
    //=========================================================================

    templates_["Root"] = { "Root",
                          "Root",
                          "Root",
                          {},
                          {{"Child", PinType::ControlOut, PinType::Control, ""}},
                          {} };

    templates_["Sequence"] = {
        "Sequence",
        "Sequence",
        "Composite",
        {
            {"In", PinType::ControlIn, PinType::ControlIn, ""} // 親からの入力
        },
        {{"Out 1", PinType::ControlOut, PinType::ControlOut, ""},
         {"Out 2", PinType::ControlOut, PinType::ControlOut, ""}} };

    templates_["Selector"] = {
        "Selector",
        "Selector",
        "Composite",
        {
            {"In", PinType::ControlIn, PinType::ControlIn, ""} // 親からの入力
        },
        {{"Out 1", PinType::ControlOut, PinType::ControlOut, ""},
         {"Out 2", PinType::ControlOut, PinType::ControlOut, ""},
         {"Out 3", PinType::ControlOut, PinType::ControlOut, ""},
         {"Out 4", PinType::ControlOut, PinType::ControlOut, ""},
         {"Out 5", PinType::ControlOut, PinType::ControlOut, ""}},
        {} };

    templates_["Inverter"] = {
        "Inverter",
        "Inverter",
        "Decorator",
        {{"In", PinType::ControlIn, PinType::ControlIn, ""}},
        {{"Out", PinType::ControlOut, PinType::ControlOut, ""}},
        {} // Output Pins
    };

    //=========================================================================
    // 移動ノード
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Action_Run";
        tmpl.displayName = "Action_Run";
        tmpl.category = "Action";

        tmpl.inputs = {
            {"In", PinType::ControlIn, PinType::ControlIn, ""},
            {"Speed", PinType::DataIn_Float, PinType::DataIn_Float, "speed"},
            {"ArrivalDist", PinType::DataIn_Float, PinType::DataIn_Float,
             "arrivalDistance"},
            {"TargetBox", PinType::DataIn_Enum, PinType::DataIn_Enum, "targetBox"} };

        tmpl.outputs = { {"Out", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = {
            {"speed", "1.0"}, {"arrivalDistance", "1.0"}, {"targetBox", "None"} };

        tmpl.propertyDefinitions = {
            {"speed", PropertyType::Float, "1.0", "", 0.0f, 10.0f, true},
            {"arrivalDistance", PropertyType::Float, "1.0", "", 0.1f, 50.0f, true},
            {"targetBox", PropertyType::Enum, "None", "TargetBoxType", 0.0f, 0.0f,
             true} };

        templates_["Action_Run"] = tmpl;
    }

    templates_["Action_Stop"] = {
        "Action_Stop",
        "Action_Stop",
        "Action",
        {{"In", PinType::ControlIn, PinType::ControlIn}},
        {{"Out", PinType::ControlOut, PinType::ControlOut}},
        {},
        {} };

    //=========================================================================
    // 攻撃データノード
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Vaiable_AttackData";
        tmpl.displayName = "Vaiable_AttackData";
        tmpl.category = "Variable";

        tmpl.inputs = {};

        tmpl.outputs = {
            {"Range", PinType::DataOut_Float, PinType::DataOut_Float, ""},
            {"Damage", PinType::DataOut_Float, PinType::DataOut_Float, ""},
            {"Cooldown", PinType::DataOut_Float, PinType::DataOut_Float, ""} };

        tmpl.defaultProperties = {
            {"range", "1.0"}, {"damage", "10.0"}, {"cooldown", "1.0"} };

        tmpl.propertyDefinitions = {
            {"range", PropertyType::Float, "3.0", "", 0.1f, 50.0f, false},
            {"damage", PropertyType::Float, "10.0", "", 0.0f, 10000.0f, false},
            {"cooldown", PropertyType::Float, "1.0", "", 0.0f, 10.0f, false} };

        templates_["Vaiable_AttackData"] = tmpl;
    }

    //=========================================================================
    // 攻撃レンジチェックノード（条件分岐）
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Condition_IsTargetInAttackRange";
        tmpl.displayName = "Condition_IsTargetInAttackRange";
        tmpl.category = "Condition";

        tmpl.inputs = {
            {"In", PinType::ControlIn, PinType::ControlIn, ""},
            {"TargetBox", PinType::DataIn_Enum, PinType::DataIn_Enum, "targetBox"},
            {"Skill", PinType::DataIn_Skill, PinType::DataIn_Skill, ""} };

        tmpl.outputs = { {"True", PinType::ControlOut, PinType::ControlOut, ""},
                        {"False", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = { {"targetBox", "Enemy"} };

        tmpl.propertyDefinitions = { {"targetBox", PropertyType::Enum, "Enemy",
                                     "TargetBoxType", 0.0f, 0.0f, true} };

        templates_["Condition_IsTargetInAttackRange"] = tmpl;
    }

    //=========================================================================
    // Action - ExecuteAttack
    //=========================================================================
    {
        NodeTemplate tmpl;
        tmpl.typeID = "Action_ExecuteAttack";
        tmpl.displayName = "Action_ExecuteAttack";
        tmpl.category = "Action";

        tmpl.inputs = {
            {"In", PinType::ControlIn, PinType::ControlIn, ""},
            {"TargetBox", PinType::DataIn_Enum, PinType::DataIn_Enum, "targetBox"},
            {"Skill", PinType::DataIn_Skill, PinType::DataIn_Skill, ""} };

        tmpl.outputs = { {"Out", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = { {"targetBox", "Enemy"} };

        tmpl.propertyDefinitions = { {"targetBox", PropertyType::Enum, "Enemy",
                                     "TargetBoxType", 0.0f, 0.0f, true} };

        templates_["Action_ExecuteAttack"] = tmpl;
    }

    //=========================================================================
    // スキルデータノード
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Data_AttackSkill";
        tmpl.displayName = "Data_AttackSkill";
        tmpl.category = "Variable";

        tmpl.inputs = {};

        tmpl.outputs = {
            {"Skill", PinType::DataOut_Skill, PinType::DataOut_Skill, ""} };

        tmpl.defaultProperties = { {"range", "2.0"},
                                  {"damage", "10.0"},
                                  {"cooldown", "1.0"},
                                  {"skillId", "0"} };

        tmpl.propertyDefinitions = {
            {"range", PropertyType::Float, "2.0", "", 0.1f, 50.0f, false},
            {"damage", PropertyType::Float, "10.0", "", 0.0f, 10000.0f, false},
            {"cooldown", PropertyType::Float, "1.0", "", 0.0f, 10.0f, false},
            {"skillId", PropertyType::Int, "0", "", 0.0f, 100.0f, false} };

        templates_["Data_AttackSkill"] = tmpl;
    }

    //=========================================================================
    // レーダースキャンノード
    //=========================================================================

    // RadarScan - 徐々に拡大するレーダースキャン
    {
        NodeTemplate tmpl;
        tmpl.typeID = "Action_RadarScan";
        tmpl.displayName = "レーダースキャン";
        tmpl.category = "Action";

        tmpl.inputs = {
            {"入力", PinType::ControlIn, PinType::ControlIn, ""},
            {"最大半径", PinType::DataIn_Float, PinType::DataIn_Float, "maxRadius"},
            {"拡大速度", PinType::DataIn_Float, PinType::DataIn_Float,
             "expandSpeed"},
            {"間隔", PinType::DataIn_Float, PinType::DataIn_Float, "scanInterval"} };

        tmpl.outputs = { {"出力", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = {
            {"maxRadius", "10.0"}, {"expandSpeed", "5.0"}, {"scanInterval", "0.1"} };

        tmpl.propertyDefinitions = {
            {"maxRadius", PropertyType::Float, "10.0", "", 1.0f, 200.0f, true},
            {"expandSpeed", PropertyType::Float, "5.0", "", 0.1f, 50.0f, true},
            {"scanInterval", PropertyType::Float, "0.1", "", 0.01f, 1.0f, true} };

        templates_["Action_RadarScan"] = tmpl;
    }

    // RadarPulse - 瞬間的なスキャン
    {
        NodeTemplate tmpl;
        tmpl.typeID = "Action_RadarPulse";
        tmpl.displayName = "Action_RadarPulse";
        tmpl.category = "Action";

        tmpl.inputs = {
            {"In", PinType::ControlIn, PinType::ControlIn, ""},
            {"Radius", PinType::DataIn_Float, PinType::DataIn_Float, "radius"} };

        tmpl.outputs = { {"Out", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = { {"radius", "10.0"} };

        tmpl.propertyDefinitions = {
            {"radius", PropertyType::Float, "10.0", "", 1.0f, 1000.0f, true} };

        templates_["Action_RadarPulse"] = tmpl;
    }

    //=========================================================================
    // 座標設定ノード
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Action_SetTargetPosition";
        tmpl.displayName = "Action_SetTargetPosition";
        tmpl.category = "Action";

        tmpl.inputs = {
            {"In", PinType::ControlIn, PinType::ControlIn, ""},
            {"X", PinType::DataIn_Float, PinType::DataIn_Float, "targetX"},
            {"Y", PinType::DataIn_Float, PinType::DataIn_Float, "targetY"},
            {"Z", PinType::DataIn_Float, PinType::DataIn_Float, "targetZ"} };

        tmpl.outputs = { {"Out", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = {
            {"targetX", "0.0"}, {"targetY", "0.0"}, {"targetZ", "0.0"} };

        tmpl.propertyDefinitions = {
            {"targetX", PropertyType::Float, "0.0", "", -1000.0f, 1000.0f, true},
            {"targetY", PropertyType::Float, "0.0", "", -1000.0f, 1000.0f, true},
            {"targetZ", PropertyType::Float, "0.0", "", -1000.0f, 1000.0f, true} };

        templates_["Action_SetTargetPosition"] = tmpl;
    }

    //=========================================================================
    // ターゲットボックスクリアノード
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Action_ClearTargetBox";
        tmpl.displayName = "対象ボックスクリア";
        tmpl.category = "Action";

        tmpl.inputs = { {"入力", PinType::ControlIn, PinType::ControlIn, ""},
                       {"ボックス種別", PinType::DataIn_Enum, PinType::DataIn_Enum,
                        "boxType"} };

        tmpl.outputs = { {"出力", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = { {"boxType", "All"} };

        tmpl.propertyDefinitions = { {"boxType", PropertyType::Enum, "All",
                                     "TargetBoxType", 0.0f, 0.0f, true} };

        templates_["Action_ClearTargetBox"] = tmpl;
    }

    //=========================================================================
    // 条件ノード - HasTargetsInBox
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Condition_HasTargetsInBox";
        tmpl.displayName = "Condition_HasTargetsInBox";
        tmpl.category = "Condition";

        tmpl.inputs = {
            {"In", PinType::ControlIn, PinType::ControlIn, ""},
            {"BoxType", PinType::DataIn_Enum, PinType::DataIn_Enum, "boxType"},
            {"MinTargets", PinType::DataIn_Int, PinType::DataIn_Int, "minTargets"} };

        tmpl.outputs = { {"True", PinType::ControlOut, PinType::ControlOut, ""},
                        {"False", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = { {"boxType", "All"}, {"minTargets", "1"} };

        tmpl.propertyDefinitions = {
            {"boxType", PropertyType::Enum, "All", "TargetBoxType", 0.0f, 0.0f,
             true},
            {"minTargets", PropertyType::Int, "1", "", 0.0f, 100.0f, true} };

        templates_["Condition_HasTargetsInBox"] = tmpl;
    }

    //=========================================================================
    // 条件ノード - IsNearTarget
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Condition_IsNearTarget";
        tmpl.displayName = "Condition_IsNearTarget";
        tmpl.category = "Condition";

        tmpl.inputs = {
            {"In", PinType::ControlIn, PinType::ControlIn, ""},
            {"BoxType", PinType::DataIn_Enum, PinType::DataIn_Enum, "boxType"},
            {"Distance", PinType::DataIn_Float, PinType::DataIn_Float, "distance"} };

        tmpl.outputs = { {"True", PinType::ControlOut, PinType::ControlOut, ""},
                        {"False", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = { {"boxType", "All"}, {"distance", "2.0"} };

        tmpl.propertyDefinitions = {
            {"boxType", PropertyType::Enum, "All", "TargetBoxType", 0.0f, 0.0f,
             true},
            {"distance", PropertyType::Float, "2.0", "", 0.1f, 100.0f, true} };

        templates_["Condition_IsNearTarget"] = tmpl;
    }

    //=========================================================================
    // 条件ノード - InTheEyes
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Condition_InTheEyes";
        tmpl.displayName = "Condition_InTheEyes";
        tmpl.category = "Condition";

        tmpl.inputs = {
            {"In", PinType::ControlIn, PinType::ControlIn, ""},
            {"TargetBox", PinType::DataIn_Enum, PinType::DataIn_Enum, "boxType"},
            {"Angle", PinType::DataIn_Float, PinType::DataIn_Float, "angle"} };

        tmpl.outputs = { {"True", PinType::ControlOut, PinType::ControlOut, ""},
                        {"False", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = { {"boxType", "Enemy"}, {"angle", "90.0"} };

        tmpl.propertyDefinitions = {
            {"boxType", PropertyType::Enum, "Enemy", "TargetBoxType", 0.0f, 0.0f,
             true},
            {"angle", PropertyType::Float, "90.0", "", 0.0f, 360.0f, true} };

        templates_["Condition_InTheEyes"] = tmpl;
    }

    //=========================================================================
    // 条件ノード - If
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Condition_If";
        tmpl.displayName = "Condition_If";
        tmpl.category = "Condition";

        tmpl.inputs = {
            {"In", PinType::ControlIn, PinType::ControlIn, ""},
            {"Condition", PinType::DataIn_Bool, PinType::DataIn_Bool, ""} };

        tmpl.outputs = { {"True", PinType::ControlOut, PinType::ControlOut, ""},
                        {"False", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = {};
        tmpl.propertyDefinitions = {};

        templates_["Condition_If"] = tmpl;
    }

    //=========================================================================
    // 変数ノード - Float
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Variable_Float";
        tmpl.displayName = "Variable_Float";
        tmpl.category = "Variable";

        tmpl.inputs = {};

        tmpl.outputs = {
            {"Value", PinType::DataOut_Float, PinType::DataOut_Float, ""} };

        tmpl.defaultProperties = { {"value", "0.0"} };

        tmpl.propertyDefinitions = {
            {"value", PropertyType::Float, "0.0", "", -10000.0f, 10000.0f, false} };

        templates_["Variable_Float"] = tmpl;
    }

    //=========================================================================
    // 変数ノード - Int
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Variable_Int";
        tmpl.displayName = "Variable_Int";
        tmpl.category = "Variable";

        tmpl.inputs = {};

        tmpl.outputs = { {"Value", PinType::DataOut_Int, PinType::DataOut_Int, ""} };

        tmpl.defaultProperties = { {"value", "0"} };

        tmpl.propertyDefinitions = {
            {"value", PropertyType::Int, "0", "", -10000.0f, 10000.0f, false} };

        templates_["Variable_Int"] = tmpl;
    }

    //=========================================================================
    // 変数ノード - Bool
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Variable_Bool";
        tmpl.displayName = "Variable_Bool";
        tmpl.category = "Variable";

        tmpl.inputs = {};

        tmpl.outputs = {
            {"Value", PinType::DataOut_Bool, PinType::DataOut_Bool, ""} };

        tmpl.defaultProperties = { {"value", "false"} };

        tmpl.propertyDefinitions = {
            {"value", PropertyType::Bool, "false", "", 0.0f, 0.0f, false} };

        templates_["Variable_Bool"] = tmpl;
    }

    //=========================================================================
    // 変数ノード - Enum (TargetBoxType)
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Variable_TargetBoxType";
        tmpl.displayName = "Variable_TargetBoxType";
        tmpl.category = "Variable";

        tmpl.inputs = {};

        tmpl.outputs = {
            {"Value", PinType::DataOut_Enum, PinType::DataOut_Enum, ""} };

        tmpl.defaultProperties = { {"value", "None"} };

        tmpl.propertyDefinitions = { {"value", PropertyType::Enum, "None",
                                     "TargetBoxType", 0.0f, 0.0f, false} };

        templates_["Variable_TargetBoxType"] = tmpl;
    }

    //=========================================================================
    // 変数ノード - CheckBox
    //=========================================================================

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Data_CheckBox";
        tmpl.displayName = "Data_CheckBox";
        tmpl.category = "Variable";

        tmpl.inputs = {};

        tmpl.outputs = {
            {"Value", PinType::DataOut_Bool, PinType::DataOut_Bool, ""} };

        tmpl.defaultProperties = { {"targetBox", "Enemy"} };

        tmpl.propertyDefinitions = { {"targetBox", PropertyType::Enum, "Enemy",
                                     "TargetBoxType", 0.0f, 0.0f, true} };

        templates_["Data_CheckBox"] = tmpl;
    }

    //=========================================================================
    // Action - GetStageTask
    //=========================================================================
    {
        NodeTemplate tmpl;
        tmpl.typeID = "Action_GetStageTask";
        tmpl.displayName = "Action_GetStageTask";
        tmpl.category = "Action";

        tmpl.inputs = { {"In", PinType::ControlIn, PinType::ControlIn, ""} };

        tmpl.outputs = { {"Out", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = {};
        tmpl.propertyDefinitions = {};

        templates_["Action_GetStageTask"] = tmpl;
    }

    //=========================================================================
    // Action - FindRouteToGoal
    //=========================================================================
    {
        NodeTemplate tmpl;
        tmpl.typeID = "Action_FindRouteToGoal";
        tmpl.displayName = "Action_FindRouteToGoal";
        tmpl.category = "Action";

        tmpl.inputs = { {"In", PinType::ControlIn, PinType::ControlIn, ""} };

        tmpl.outputs = { {"Out", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = {};
        tmpl.propertyDefinitions = {};

        templates_["Action_FindRouteToGoal"] = tmpl;
    }

    //=========================================================================
    // Action - DetectObstacle
    //=========================================================================
    {
        NodeTemplate tmpl;
        tmpl.typeID = "Action_DetectObstacle";
        tmpl.displayName = "Action_DetectObstacle";
        tmpl.category = "Action";

        tmpl.inputs = { {"In", PinType::ControlIn, PinType::ControlIn, ""},
                       {"CheckDist", PinType::DataIn_Float, PinType::DataIn_Float,
                        "checkDistance"},
                       {"CheckRadius", PinType::DataIn_Float, PinType::DataIn_Float,
                        "checkRadius"} };

        tmpl.outputs = { {"Out", PinType::ControlOut, PinType::ControlOut, ""} };

        tmpl.defaultProperties = { {"checkDistance", "5.0"}, {"checkRadius", "1.0"} };

        tmpl.propertyDefinitions = {
            {"checkDistance", PropertyType::Float, "5.0", "", 0.1f, 50.0f, true},
            {"checkRadius", PropertyType::Float, "1.0", "", 0.1f, 10.0f, true} };

        templates_["Action_DetectObstacle"] = tmpl;
    }

    {
        NodeTemplate tmpl;
        tmpl.typeID = "Variable_Skill_SwordSlash";
        tmpl.displayName = "Variable_Skill_SwordSlash";
        tmpl.category = "Variable";

        tmpl.inputs = {};
        tmpl.outputs = {
            {"SkillID", PinType::DataOut_Skill, PinType::DataOut_Skill, ""} };

        tmpl.defaultProperties = { {"range", "1.5"},
                                  {"damage", "15.0"},
                                  {"cooldown", "1.0"},
                                  {"skillId", "2"} };

        tmpl.propertyDefinitions = {
            {"range", PropertyType::Float, "1.5", "", 0.1f, 50.0f, false, false},
            {"damage", PropertyType::Float, "15.0", "", 0.0f, 10000.0f, false,
             false},
            {"cooldown", PropertyType::Float, "1.0", "", 0.0f, 10.0f, false, false},
            {"skillId", PropertyType::Int, "2", "", 0.0f, 100.0f, false, false} };

        templates_["Variable_Skill_SwordSlash"] = tmpl;
    }


    // < (Less)
    {
        NodeTemplate tmpl;
        tmpl.typeID = "Data_Less";
        tmpl.displayName = "Data_Less";
        tmpl.category = "Logic";
        tmpl.inputs = {
            {"OperandA", PinType::DataIn_Generic, PinType::DataIn_Generic, ""},
            {"OperandB", PinType::DataIn_Generic, PinType::DataIn_Generic, ""} };
        tmpl.outputs = {
            {"Result", PinType::DataOut_Bool, PinType::DataOut_Bool, ""} };
        tmpl.defaultProperties = {};
        tmpl.propertyDefinitions = {};
        templates_["Data_Less"] = tmpl;
    }

    // > (Greater)
    {
        NodeTemplate tmpl;
        tmpl.typeID = "Data_Greater";
        tmpl.displayName = "Data_Greater";
        tmpl.category = "Logic";
        tmpl.inputs = {
            {"OperandA", PinType::DataIn_Generic, PinType::DataIn_Generic, ""},
            {"OperandB", PinType::DataIn_Generic, PinType::DataIn_Generic, ""} };
        tmpl.outputs = {
            {"Result", PinType::DataOut_Bool, PinType::DataOut_Bool, ""} };
        tmpl.defaultProperties = {};
        tmpl.propertyDefinitions = {};
        templates_["Data_Greater"] = tmpl;
    }

    // <= (LessEqual)
    {
        NodeTemplate tmpl;
        tmpl.typeID = "Data_LessEqual";
        tmpl.displayName = "Data_LessEqual";
        tmpl.category = "Logic";
        tmpl.inputs = {
            {"OperandA", PinType::DataIn_Generic, PinType::DataIn_Generic, ""},
            {"OperandB", PinType::DataIn_Generic, PinType::DataIn_Generic, ""} };
        tmpl.outputs = {
            {"Result", PinType::DataOut_Bool, PinType::DataOut_Bool, ""} };
        tmpl.defaultProperties = {};
        tmpl.propertyDefinitions = {};
        templates_["Data_LessEqual"] = tmpl;
    }

    // >= (GreaterEqual)
    {
        NodeTemplate tmpl;
        tmpl.typeID = "Data_GreaterEqual";
        tmpl.displayName = "Data_GreaterEqual";
        tmpl.category = "Logic";
        tmpl.inputs = {
            {"OperandA", PinType::DataIn_Generic, PinType::DataIn_Generic, ""},
            {"OperandB", PinType::DataIn_Generic, PinType::DataIn_Generic, ""} };
        tmpl.outputs = {
            {"Result", PinType::DataOut_Bool, PinType::DataOut_Bool, ""} };
        tmpl.defaultProperties = {};
        tmpl.propertyDefinitions = {};
        templates_["Data_GreaterEqual"] = tmpl;
    }

    // == (Equal)
    {
        NodeTemplate tmpl;
        tmpl.typeID = "Data_Equal";
        tmpl.displayName = "Data_Equal";
        tmpl.category = "Logic";
        tmpl.inputs = {
            {"OperandA", PinType::DataIn_Generic, PinType::DataIn_Generic, ""},
            {"OperandB", PinType::DataIn_Generic, PinType::DataIn_Generic, ""} };
        tmpl.outputs = {
            {"Result", PinType::DataOut_Bool, PinType::DataOut_Bool, ""} };
        tmpl.defaultProperties = {};
        tmpl.propertyDefinitions = {};
        templates_["Data_Equal"] = tmpl;
    }

    // != (NotEqual)
    {
        NodeTemplate tmpl;
        tmpl.typeID = "Data_NotEqual";
        tmpl.displayName = "Data_NotEqual";
        tmpl.category = "Logic";
        tmpl.inputs = {
            {"OperandA", PinType::DataIn_Generic, PinType::DataIn_Generic, ""},
            {"OperandB", PinType::DataIn_Generic, PinType::DataIn_Generic, ""} };
        tmpl.outputs = {
            {"Result", PinType::DataOut_Bool, PinType::DataOut_Bool, ""} };
        tmpl.defaultProperties = {};
        tmpl.propertyDefinitions = {};
        templates_["Data_NotEqual"] = tmpl;
    }

    std::cout << "INFO: Registered " << templates_.size()
        << " built-in behavior nodes." << std::endl;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスの生成
std::unique_ptr<NodeInstance>
NodeFactory::CreateNode(const std::string& templateName,
    const VECTOR2& position, long long& nextNodeId) {
    if (templates_.find(templateName) == templates_.end()) {
        std::cerr << "ERROR: Node template '" << templateName << "' not found."
            << std::endl;
        return nullptr;
    }

    const NodeTemplate& tmpl = templates_.at(templateName);
    auto node = std::make_unique<NodeInstance>();

    // ユニークIDの生成
    node->id =
        "Node_" +
        std::to_string(nextNodeId++); // IDを割り当て、カウンターをインクリメント
    node->typeId = templateName;
    node->displayName = tmpl.displayName;
    node->position = position;

    // ピンの初期化とID付与
    InitializeNodePins(*node, tmpl);

    // プロパティの初期化
    node->properties = tmpl.defaultProperties;

    for (const auto& propDef : tmpl.propertyDefinitions) {
        node->propertyDefinitions[propDef.name] = propDef;
    }

    return node;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードのピンを初期化し、ユニークIDを付与
void NodeFactory::InitializeNodePins(NodeInstance& node,
    const NodeTemplate& tmpl) {
    // PinIDの生成とPinInstanceの作成
    auto createPinInstance = [&](const PinDefinition& def,
        const std::string& parentId) -> PinInstance {
            PinInstance pin;

            // ユニークなPinIDを生成 (NodeID_Pin_Counter)
            pin.id = parentId + "_Pin_" + std::to_string(nextPinId_++);
            pin.name = def.name;
            pin.type = def.type;
            pin.parentNodeId = parentId;
            // positionはCanvasが描画時に計算
            return pin;
        };

    for (const auto& def : tmpl.inputs) {
        node.inputPins.push_back(createPinInstance(def, node.id));
    }

    for (const auto& def : tmpl.outputs) {
        node.outputPins.push_back(createPinInstance(def, node.id));
    }
}
//-----------------------------------------------------------------------------