#include "Serializer.h"
#include <fstream>
#include <iostream>

//-----------------------------------------------------------------------------
// グラフデータをJSON形式で保存
std::string Serializer::PinTypeToString(PinType type) const {
    // PinTypeを文字列に変換
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
    case PinType::DataOut_String:
        return "DataOut_String";
    case PinType::DataIn_Bool:
        return "DataIn_Bool";
    case PinType::DataOut_Bool:
        return "DataOut_Bool";
    case PinType::DataIn_Enum:
        return "DataIn_Enum";
    case PinType::DataOut_Enum:
        return "DataOut_Enum";
    case PinType::DataIn_Generic:
        return "DataIn_Generic";
    case PinType::DataOut_Generic:
        return "DataOut_Generic";
    }
    return "Unknown";
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PinTypeを文字列から変換
PinType Serializer::StringToPinType(const std::string& str) const {
	// 文字列をPinTypeに変換
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
    return PinType::ControlIn;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PropertyTypeを文字列に変換
std::string Serializer::PropertyTypeToString(PropertyType type) const {
	// PropertyTypeを文字列に変換
    switch (type) {
    case PropertyType::Float:
        return "Float";
    case PropertyType::Int:
        return "Int";
    case PropertyType::Bool:
        return "Bool";
    case PropertyType::String:
        return "String";
    case PropertyType::Enum:
        return "Enum";
    case PropertyType::Vector3:
        return "Vector3";
    default:
        return "Float";
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
PropertyType Serializer::StringToPropertyType(const std::string& str) const {
	// 文字列をPropertyTypeに変換
    if (str == "Float")
        return PropertyType::Float;
    if (str == "Int")
        return PropertyType::Int;
    if (str == "Bool")
        return PropertyType::Bool;
    if (str == "String")
        return PropertyType::String;
    if (str == "Enum")
        return PropertyType::Enum;
    if (str == "Vector3")
        return PropertyType::Vector3;
    return PropertyType::Float;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// --- JSON変換のためのADLフリー関数 ---
namespace {
    std::string PinTypeToString_ADL(PinType type) {
        return Serializer().PinTypeToString(type);
    }
    PinType StringToPinType_ADL(const std::string& str) {
        return Serializer().StringToPinType(str);
    }
    std::string PropertyTypeToString_ADL(PropertyType type) {
        return Serializer().PropertyTypeToString(type);
    }
    PropertyType StringToPropertyType_ADL(const std::string& str) {
        return Serializer().StringToPropertyType(str);
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// --- 1. VECTOR2 ---
void to_json(json& j, const VECTOR2& p) { j = json{ {"x", p.x}, {"y", p.y} }; }
void from_json(const json& j, VECTOR2& p) {
    if (j.contains("x"))
        j.at("x").get_to(p.x);
    if (j.contains("y"))
        j.at("y").get_to(p.y);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// --- 2. PinInstance ---
void to_json(json& j, const PinInstance& p) {
    j = json{
        {"id", p.id},
        {"name", p.name},
        {"type", PinTypeToString_ADL(p.type)},
        {"parentNodeId", p.parentNodeId},
        {"linkedPinIds", p.linkedPinIds},
        {"linkedPropertyName", p.linkedPropertyName}, // プロパティ紐付けを追加
        {"dataValue", p.dataValue}                    // データ値も保存
    };
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PinInstanceのJSON変換関数
void from_json(const json& j, PinInstance& p) {
	// JSONオブジェクトからPinInstanceのメンバーに値を設定
    if (j.contains("id"))
        j.at("id").get_to(p.id);
    if (j.contains("name"))
        j.at("name").get_to(p.name);
    if (j.contains("type"))
        p.type = StringToPinType_ADL(j.at("type").get<std::string>());
    if (j.contains("parentNodeId"))
        j.at("parentNodeId").get_to(p.parentNodeId);
    if (j.contains("linkedPinIds"))
        j.at("linkedPinIds").get_to(p.linkedPinIds);
    if (j.contains("linkedPropertyName"))
        j.at("linkedPropertyName").get_to(p.linkedPropertyName);
    if (j.contains("dataValue"))
        j.at("dataValue").get_to(p.dataValue);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// --- 3. LinkInstance ---
void to_json(json& j, const LinkInstance& l) {
    j = json{
        {"id", l.id}, {"startPinId", l.startPinId}, {"endPinId", l.endPinId} };
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// LinkInstanceのJSON変換関数
void from_json(const json& j, LinkInstance& l) {
	// JSONオブジェクトからLinkInstanceのメンバーに値を設定
    if (j.contains("id"))
        j.at("id").get_to(l.id);
    if (j.contains("startPinId"))
        j.at("startPinId").get_to(l.startPinId);
    if (j.contains("endPinId"))
        j.at("endPinId").get_to(l.endPinId);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// --- 4. PropertyDefinition ---
void to_json(json& j, const PropertyDefinition& p) {
	// JSONオブジェクトにPropertyDefinitionのメンバーを設定
    j = json{ {"name", p.name},
             {"type", PropertyTypeToString_ADL(p.type)},
             {"defaultValue", p.defaultValue},
             {"enumTypeName", p.enumTypeName},
             {"minValue", p.minValue},
             {"maxValue", p.maxValue},
             {"hasDataPin", p.hasDataPin} };
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PropertyDefinitionのJSON変換関数
void from_json(const json& j, PropertyDefinition& p) {
	// JSONオブジェクトからPropertyDefinitionのメンバーに値を設定
    if (j.contains("name"))
        j.at("name").get_to(p.name);
    if (j.contains("type"))
        p.type = StringToPropertyType_ADL(j.at("type").get<std::string>());
    if (j.contains("defaultValue"))
        j.at("defaultValue").get_to(p.defaultValue);
    if (j.contains("enumTypeName"))
        j.at("enumTypeName").get_to(p.enumTypeName);
    if (j.contains("minValue"))
        j.at("minValue").get_to(p.minValue);
    if (j.contains("maxValue"))
        j.at("maxValue").get_to(p.maxValue);
    if (j.contains("hasDataPin"))
        j.at("hasDataPin").get_to(p.hasDataPin);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// --- 5. NodeInstance ---
void to_json(json& j, const NodeInstance& n) {
	// propertyDefinitionsはマップなので、シリアライズのためにベクターに変換
    std::vector<PropertyDefinition> propDefs;
    for (const auto& pair : n.propertyDefinitions) {
        propDefs.push_back(pair.second);
    }

	// JSONオブジェクトにNodeInstanceのメンバーを設定
    j = json{ {"id", n.id},
             {"IdName", n.typeId},
             {"displayName", n.displayName},
             {"position", n.position},
             {"inputPins", n.inputPins},
             {"outputPins", n.outputPins},
             {"properties", n.properties},
             {"propertyDefinitions", propDefs} };
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// NodeInstanceのJSON変換関数
void from_json(const json& j, NodeInstance& n) {
	// JSONオブジェクトからNodeInstanceのメンバーに値を設定
    if (j.contains("id"))
        j.at("id").get_to(n.id);
    if (j.contains("IdName"))
        j.at("IdName").get_to(n.typeId);
    if (j.contains("displayName"))
        j.at("displayName").get_to(n.displayName);
    if (j.contains("position"))
        j.at("position").get_to(n.position);
    if (j.contains("inputPins"))
        j.at("inputPins").get_to(n.inputPins);
    if (j.contains("outputPins"))
        j.at("outputPins").get_to(n.outputPins);
    if (j.contains("properties"))
        j.at("properties").get_to(n.properties);

    // propertyDefinitionsをマップに復元
    if (j.contains("propertyDefinitions")) {
        std::vector<PropertyDefinition> propDefs;
        j.at("propertyDefinitions").get_to(propDefs);
        for (const auto& def : propDefs) {
            n.propertyDefinitions[def.name] = def;
        }
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// --- 6. GraphData (ルートオブジェクト) ---
void to_json(json& j, const GraphData& g) {
    j = json{ {"nextNodeId", g.nextNodeId},
             {"nextLinkId", g.nextLinkId},
             {"nodes", g.nodes},
             {"links", g.links},
             {"allPins", g.allPins} };
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// GraphDataのJSON変換関数
void from_json(const json& j, GraphData& g) {
	// JSONオブジェクトからGraphDataのメンバーに値を設定
    if (j.contains("nextNodeId"))
        j.at("nextNodeId").get_to(g.nextNodeId);
    if (j.contains("nextLinkId"))
        j.at("nextLinkId").get_to(g.nextLinkId);
    if (j.contains("nodes"))
        j.at("nodes").get_to(g.nodes);
    if (j.contains("links"))
        j.at("links").get_to(g.links);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// グラフデータをJSON形式で保存
bool Serializer::Save(const GraphData& graphData,
    const std::string& filePath) const {
	// JSONオブジェクトにGraphDataをシリアライズしてファイルに保存
    try {
        json j = graphData;

        std::ofstream ofs(filePath);
        if (!ofs.is_open()) {
            std::cerr << "ERROR: Failed to open file for writing: " << filePath
                << std::endl;
            return false;
        }

        ofs << j.dump(4);
        ofs.close();

        std::cout << "INFO: Graph saved successfully to " << filePath << std::endl;
        return true;

    }
    catch (const json::exception& e) {
        std::cerr << "ERROR: JSON serialization error: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: Standard exception during serialization: " << e.what()
            << std::endl;
        return false;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// グラフデータをJSON形式で読み込み
bool Serializer::Load(const std::string& filePath, GraphData& graphData) {
	// JSONファイルからGraphDataをデシリアライズして読み込む
    try {
        std::ifstream ifs(filePath);
        if (!ifs.is_open()) {
            std::cerr << "ERROR: Failed to open file for reading: " << filePath
                << std::endl;
            return false;
        }

        json j;
        ifs >> j;
        ifs.close();

        graphData = j.get<GraphData>();

        std::cout << "INFO: Graph loaded successfully from " << filePath
            << std::endl;
        return true;

    }
    catch (const json::exception& e) {
        std::cerr << "ERROR: JSON deserialization error (file may be corrupted): "
            << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: Standard exception during deserialization: "
            << e.what() << std::endl;
        return false;
    }
}
//-----------------------------------------------------------------------------