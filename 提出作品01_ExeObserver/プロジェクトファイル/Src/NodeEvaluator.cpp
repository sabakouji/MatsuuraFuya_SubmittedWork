#include "NodeEvaluator.h"
#include "Executor.h"
#include <iostream>

//-----------------------------------------------------------------------------
// ノードの入力ピンから接続されているノードを取得
const NodeInstance* NodeEvaluator::GetInputNode(const NodeInstance* node,
    const std::string& pinName,
    Executor* executor) {
	// 安全チェック
    if (!node || !executor)
        return nullptr;

	// Executorから、nodeのpinNameに接続されているノードIDを取得
    std::string sourceId = executor->GetInputSourceNodeId(node->id, pinName);

    if (sourceId.empty())
        return nullptr;

	// Executorから、sourceIdに対応するNodeInstanceを取得
    return executor->GetNodeInstance(sourceId);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードの評価結果をboolとして返す
bool NodeEvaluator::EvaluateBool(const NodeInstance* node, Executor* executor) {
	// 安全チェック
    if (!node || !executor)
        return false;
	// ノードのtypeIdに応じて評価方法を分岐
    const std::string& typeId = node->typeId;

	// 1. Variable_Bool: プロパティ "value" を "true"/"false" として評価
    if (typeId == "Variable_Bool") {
        auto it = node->properties.find("value");
        if (it != node->properties.end()) {
            return (it->second == "true");
        }
        return false;
    }

	// 2. Data_CheckBox: プロパティ "targetBox" を TargetBoxType の enum として評価し、ExecutorのHasTargetsInBoxで判定
    if (typeId == "Data_CheckBox") {
        TargetBoxType boxType = TargetBoxType::Enemy;
        auto it = node->properties.find("targetBox");
        if (it != node->properties.end()) {
            const auto* enumDef =
                EnumRegistry::GetInstance().GetEnum("TargetBoxType");
            if (enumDef) {
                boxType = static_cast<TargetBoxType>(enumDef->GetIndex(it->second));
            }
        }
        return executor->HasTargetsInBox(boxType);
    }

	// 3. Comparison Nodes: "Data_Less", "Data_Greater", "Data_LessEqual", "Data_GreaterEqual", "Data_Equal", "Data_NotEqual"
    if (typeId == "Data_Less" || typeId == "Data_Greater" ||
        typeId == "Data_LessEqual" || typeId == "Data_GreaterEqual" ||
        typeId == "Data_Equal" || typeId == "Data_NotEqual") {

        const NodeInstance* lhsNode = GetInputNode(node, "OperandA", executor);
        const NodeInstance* rhsNode = GetInputNode(node, "OperandB", executor);

        if (!lhsNode || !rhsNode) {
            return false;
        }

        GenericValue lhsVal = EvaluateGeneric(lhsNode, executor);
        GenericValue rhsVal = EvaluateGeneric(rhsNode, executor);

        if (typeId == "Data_Less")
            return CompareValues(lhsVal, rhsVal, "<");
        if (typeId == "Data_Greater")
            return CompareValues(lhsVal, rhsVal, ">");
        if (typeId == "Data_LessEqual")
            return CompareValues(lhsVal, rhsVal, "<=");
        if (typeId == "Data_GreaterEqual")
            return CompareValues(lhsVal, rhsVal, ">=");
        if (typeId == "Data_Equal")
            return CompareValues(lhsVal, rhsVal, "==");
        if (typeId == "Data_NotEqual")
            return CompareValues(lhsVal, rhsVal, "!=");
    }

    return false;
}


float NodeEvaluator::EvaluateFloat(const NodeInstance* node,
    Executor* executor) {
    if (!node || !executor)
        return 0.0f;

    if (node->typeId == "Variable_Float") {
        auto it = node->properties.find("value");
        if (it != node->properties.end()) {
            try {
                return std::stof(it->second);
            }
            catch (...) {
                return 0.0f;
            }
        }
    }
    return 0.0f;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードの評価結果をintとして返す
int NodeEvaluator::EvaluateInt(const NodeInstance* node, Executor* executor) {
	// 安全チェック
    if (!node || !executor)
        return 0;

	// Variable_Int: プロパティ "value" を整数として評価
    if (node->typeId == "Variable_Int") {
        auto it = node->properties.find("value");
        if (it != node->properties.end()) {
            try {
                return std::stoi(it->second);
            }
            catch (...) {
                return 0;
            }
        }
    }
    return 0;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードの評価結果をstringとして返す
std::string NodeEvaluator::EvaluateString(const NodeInstance* node,
    Executor* executor) {
	// 安全チェック
    if (!node || !executor)
        return "";

	// Variable_String: プロパティ "value" を文字列として評価 (仮に追加された場合)
    if (node->typeId == "Variable_String") { // Hypothetical, if added
        auto it = node->properties.find("value");
        if (it != node->properties.end()) {
            return it->second;
        }
    }
    return "";
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードの評価結果をGenericValueとして返す
GenericValue NodeEvaluator::EvaluateGeneric(const NodeInstance* node,
    Executor* executor) {
	// 安全チェック
    if (!node)
        return 0.0f; // Default

	// 1. Variable_Float, Variable_Int, Variable_Bool: プロパティ "value" を対応する型として評価
    if (node->typeId == "Variable_Float")
        return EvaluateFloat(node, executor);
    if (node->typeId == "Variable_Int")
        return EvaluateInt(node, executor);
    if (node->typeId == "Variable_Bool")
        return EvaluateBool(node, executor);
    if (node->typeId == "Data_CheckBox")
        return EvaluateBool(node, executor);
    if (node->typeId == "Data_Less")
        return EvaluateBool(node, executor);
    if (node->typeId == "Data_Greater")
        return EvaluateBool(node, executor);
    if (node->typeId == "Data_LessEqual")
        return EvaluateBool(node, executor);
    if (node->typeId == "Data_GreaterEqual")
        return EvaluateBool(node, executor);
    if (node->typeId == "Data_Equal")
        return EvaluateBool(node, executor);
    if (node->typeId == "Data_NotEqual")
        return EvaluateBool(node, executor);

    return 0.0f; 
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 2つのGenericValueを比較する
bool NodeEvaluator::CompareValues(const GenericValue& lhs,
    const GenericValue& rhs,
    const std::string& op) {
	// 型を判定して比較
    bool lhsIsFloat = std::holds_alternative<float>(lhs);
    bool lhsIsInt = std::holds_alternative<int>(lhs);
    bool rhsIsFloat = std::holds_alternative<float>(rhs);
    bool rhsIsInt = std::holds_alternative<int>(rhs);

	// 数値同士の比較は、両方ともfloatかintであれば、floatにキャストして比較する
    if ((lhsIsFloat || lhsIsInt) && (rhsIsFloat || rhsIsInt)) {
        float l = lhsIsFloat ? std::get<float>(lhs) : (float)std::get<int>(lhs);
        float r = rhsIsFloat ? std::get<float>(rhs) : (float)std::get<int>(rhs);

        if (op == "<")
            return l < r;
        if (op == ">")
            return l > r;
        if (op == "<=")
            return l <= r;
        if (op == ">=")
            return l >= r;
        if (op == "==")
            return std::abs(l - r) < 0.0001f;
        if (op == "!=")
            return std::abs(l - r) >= 0.0001f;
    }

	// 文字列同士の比較は、両方ともstringであれば、stringとして比較する
    if (std::holds_alternative<bool>(lhs) && std::holds_alternative<bool>(rhs)) {
        bool l = std::get<bool>(lhs);
        bool r = std::get<bool>(rhs);

        if (op == "==")
            return l == r;
        if (op == "!=")
            return l != r;
    }

    return false;
}
//-----------------------------------------------------------------------------