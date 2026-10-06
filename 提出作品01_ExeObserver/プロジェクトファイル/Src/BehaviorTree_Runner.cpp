#include "BehaviorTree_Runner.h"
#include "BehaviorTree_Actions.h"
#include "BehaviorTree_Nodes.h"
#include "Goal.h"
#include "NodeValueRegistry.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <sstream>

using Nodeptr = BehaviorTreeNode::Nodeptr;

//---------------------------------------------------------------
// TargetBoxTypeの文字列変換
TargetBoxType StringToTargetBoxType(const std::string &str) {
	// 文字列を小文字に変換して比較
    if (str == "None")
        return TargetBoxType::None;
    if (str == "All")
        return TargetBoxType::All;
    if (str == "Enemy")
        return TargetBoxType::Enemy;
    if (str == "Door")
        return TargetBoxType::Door;
    if (str == "Item")
        return TargetBoxType::Item;
    if (str == "Goal")
        return TargetBoxType::Goal;
    if (str == "Position")
        return TargetBoxType::Position;

    // デフォルトはNone
    std::cerr << "WARNING: Unknown TargetBoxType: " << str << ". Using None."
        << std::endl;
    return TargetBoxType::None;
}
//---------------------------------------------------------------

//---------------------------------------------------------------
// TargetBoxTypeの文字列変換
const char *TargetBoxTypeToString(TargetBoxType type) {
	// TargetBoxTypeの文字列変換
    switch (type) {
    case TargetBoxType::None:
        return "None";
    case TargetBoxType::All:
        return "All";
    case TargetBoxType::Enemy:
        return "Enemy";
    case TargetBoxType::Door:
        return "Door";
    case TargetBoxType::Item:
        return "Item";
    case TargetBoxType::Goal:
        return "Goal";
    case TargetBoxType::Position:
        return "Position";
    default:
        return "Unknown";
    }
}
//---------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードIDからNodeInstanceを検索
const NodeInstance *
BehaviorTreeRunner::FindNodeInctanceById(const std::string &nodeId) const {
	// ノードIDからNodeInstanceを検索
    for (const auto& node : m_graphdata.nodes) {
        if (node.id == nodeId) {
            return &node;
        }
    }
    return nullptr;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ピンIDからPinInstanceを検索
const PinInstance *
BehaviorTreeRunner::FindPinInstance(const std::string &pinId) const {
	// ピンIDからPinInstanceを検索
    auto it = m_graphdata.allPins.find(pinId);
    if (it != m_graphdata.allPins.end())
        return &it->second;
    return nullptr;
}

//-----------------------------------------------------------------------------
// ノードインスタンスからビヘイビアノードを生成
Nodeptr
BehaviorTreeRunner::CreateBehaviorNode(const NodeInstance &nodeInstance) {
    const auto& typeId = nodeInstance.typeId;

    //=========================================================================
    // 基本ノード
    //=========================================================================

	// ルートノード
    if (typeId == "Root") {
        return std::make_unique<RootNode>(nodeInstance, nodeInstance.displayName);
    }

	// コンポジットノード
    else if (typeId == "Selector") {
        return std::make_unique<SelectorNode>(nodeInstance,
            nodeInstance.displayName);
    }
	// シーケンスノード
    else if (typeId == "Sequence") {
        return std::make_unique<SequenceNode>(nodeInstance,
            nodeInstance.displayName);
    }

    //=========================================================================
    // アクションノード
    //=========================================================================

	// 前方移動ノード
    else if (typeId == "Action_Run") {
        float speed = 1.0f;
        float arrivalDistance = 1.0f;
        TargetBoxType targetBox = TargetBoxType::None;

        auto itSpeed = nodeInstance.properties.find("speed");
        if (itSpeed != nodeInstance.properties.end()) {
            speed = std::stof(itSpeed->second);
        }

        auto itDist = nodeInstance.properties.find("arrivalDistance");
        if (itDist != nodeInstance.properties.end()) {
            arrivalDistance = std::stof(itDist->second);
        }

        auto itBox = nodeInstance.properties.find("targetBox");
        if (itBox != nodeInstance.properties.end()) {
            targetBox = StringToTargetBoxType(itBox->second);
        }

        std::cout << "INFO: Creating RunAction - Speed=" << speed
            << ", ArrivalDistance=" << arrivalDistance
            << ", TargetBox=" << TargetBoxTypeToString(targetBox)
            << std::endl;

        return std::make_unique<RunAction>(nodeInstance, speed, arrivalDistance,
            targetBox, nodeInstance.displayName);
    }

	// 停止ノード
    else if (typeId == "Action_Stop") {
        return std::make_unique<StopAction>(nodeInstance, nodeInstance.displayName);
    }

    //=========================================================================
    // レーダースキャンノード
    //=========================================================================

	// レーダースキャンノード - 徐々に半径を広げながらスキャンを実行し、種類別のボックスに分類
    else if (typeId == "Action_RadarScan") {
        float maxRadius = 10.0f;
        float expandSpeed = 5.0f;
        float scanInterval = 0.1f;

        auto itRadius = nodeInstance.properties.find("maxRadius");
        if (itRadius != nodeInstance.properties.end())
            maxRadius = std::stof(itRadius->second);

        auto itSpeed = nodeInstance.properties.find("expandSpeed");
        if (itSpeed != nodeInstance.properties.end())
            expandSpeed = std::stof(itSpeed->second);

        auto itInterval = nodeInstance.properties.find("scanInterval");
        if (itInterval != nodeInstance.properties.end())
            scanInterval = std::stof(itInterval->second);

        std::cout << "INFO: Creating RadarScanAction - MaxRadius=" << maxRadius
            << std::endl;

        return std::make_unique<RadarScanAction>(nodeInstance, maxRadius,
            expandSpeed, scanInterval,
            nodeInstance.displayName);
    }

	// レーダーパルスノード - 一瞬で指定半径内をスキャンして種類別のボックスに分類
    else if (typeId == "Action_RadarPulse") {
        float radius = 10.0f;

        auto itRadius = nodeInstance.properties.find("radius");
        if (itRadius != nodeInstance.properties.end())
            radius = std::stof(itRadius->second);

        std::cout << "INFO: Creating RadarPulseAction - Radius=" << radius
            << std::endl;

        return std::make_unique<RadarPulseAction>(nodeInstance, radius,
            nodeInstance.displayName);
    }

    //=========================================================================
    // ターゲットボックス操作ノード
    //=========================================================================

	// ターゲット位置設定ノード - 手動でターゲット位置を指定
    else if (typeId == "Action_SetTargetPosition") {
        float x = 0.0f, y = 0.0f, z = 0.0f;

        auto itX = nodeInstance.properties.find("targetX");
        if (itX != nodeInstance.properties.end())
            x = std::stof(itX->second);

        auto itY = nodeInstance.properties.find("targetY");
        if (itY != nodeInstance.properties.end())
            y = std::stof(itY->second);

        auto itZ = nodeInstance.properties.find("targetZ");
        if (itZ != nodeInstance.properties.end())
            z = std::stof(itZ->second);

        return std::make_unique<SetTargetPositionAction>(nodeInstance, x, y, z,
            nodeInstance.displayName);
    }

	// ターゲットボックスクリアノード - 指定した種類のターゲットボックスをクリア
    else if (typeId == "Action_ClearTargetBox") {
        TargetBoxType boxType = TargetBoxType::All;

        auto itBox = nodeInstance.properties.find("boxType");
        if (itBox != nodeInstance.properties.end())
            boxType = StringToTargetBoxType(itBox->second);

        return std::make_unique<ClearTargetBoxAction>(nodeInstance, boxType,
            nodeInstance.displayName);
    }

    //=========================================================================
    // 条件ノード
    //=========================================================================

	// ターゲットボックス内のターゲット数チェックノード - 指定した種類のターゲットボックス内に、指定数以上のターゲットが存在するか
    else if (typeId == "Condition_HasTargetsInBox") {
        TargetBoxType boxType = TargetBoxType::All;
        int minTargets = 1;

        auto itBox = nodeInstance.properties.find("boxType");
        if (itBox != nodeInstance.properties.end())
            boxType = StringToTargetBoxType(itBox->second);

        auto itMin = nodeInstance.properties.find("minTargets");
        if (itMin != nodeInstance.properties.end())
            minTargets = std::stoi(itMin->second);

        return std::make_unique<HasTargetsInBoxCondition>(
            nodeInstance, boxType, minTargets, nodeInstance.displayName);
    }

	// ターゲットに近いかチェックノード - 指定した種類のターゲットボックス内の最も近いターゲットが、指定距離以内に存在するか
    else if (typeId == "Condition_IsNearTarget") {
        TargetBoxType boxType = TargetBoxType::All;
        float distance = 2.0f;

        auto itBox = nodeInstance.properties.find("boxType");
        if (itBox != nodeInstance.properties.end())
            boxType = StringToTargetBoxType(itBox->second);

        auto itDist = nodeInstance.properties.find("distance");
        if (itDist != nodeInstance.properties.end())
            distance = std::stof(itDist->second);

        return std::make_unique<IsNearTargetCondition>(
            nodeInstance, boxType, distance, nodeInstance.displayName);
    }

	// ターゲットの正面にいるかチェックノード - 指定した種類のターゲットボックス内の最も近いターゲットが、指定角度範囲内の正面に存在するか
    else if (typeId == "Condition_InTheEyes") {
        TargetBoxType boxType = TargetBoxType::Enemy;
        float angle = 90.0f;

        auto itBox = nodeInstance.properties.find("boxType");
        if (itBox != nodeInstance.properties.end())
            boxType = StringToTargetBoxType(itBox->second);

        auto itAngle = nodeInstance.properties.find("angle");
        if (itAngle != nodeInstance.properties.end())
            angle = std::stof(itAngle->second);

        return std::make_unique<InTheEyesCondition>(nodeInstance, boxType, angle,
            nodeInstance.displayName);
    }

    //=========================================================================
    // 攻撃関連ノード
    //=========================================================================

	// 攻撃範囲内にいるかチェックノード - 指定した種類のターゲットボックス内の最も近いターゲットが、攻撃範囲内に存在するか
    else if (typeId == "Condition_IsTargetInAttackRange") {
        TargetBoxType targetType = TargetBoxType::Enemy;

        auto itTargetType = nodeInstance.properties.find("targetBox");
        if (itTargetType != nodeInstance.properties.end()) {
            targetType = StringToTargetBoxType(itTargetType->second);
        }

        return std::make_unique<CheckAttackRangeCondition>(
            nodeInstance, targetType, nodeInstance.displayName);
    }

	// 条件分岐ノード - 条件の結果によって、TrueノードまたはFalseノードに分岐
    else if (typeId == "Condition_If") {
        return std::make_unique<IfNode>(nodeInstance, nodeInstance.displayName);
    }
	// 攻撃実行ノード - 指定した種類のターゲットボックス内のターゲットに対して攻撃を実行
    else if (typeId == "Action_ExecuteAttack") {
        float Range = 3.0f;
        float Damege = 100.0f;
        TargetBoxType targetType = TargetBoxType::Enemy;

        auto itRange = nodeInstance.properties.find("attackRange");
        if (itRange != nodeInstance.properties.end()) {
            Range = std::stof(itRange->second);
        }

        auto itDamage = nodeInstance.properties.find("attackDamage");
        if (itDamage != nodeInstance.properties.end()) {
            Damege = std::stof(itDamage->second);
        }

        auto itTargetType = nodeInstance.properties.find("targetBox");
        if (itTargetType != nodeInstance.properties.end()) {
            targetType = StringToTargetBoxType(itTargetType->second);
        }

        return std::make_unique<ExecuteAttackAction>(nodeInstance, targetType,
            nodeInstance.displayName);
    }
	// 攻撃スキルノード - 指定した攻撃スキルを実行
    else if (typeId == "Data_AttackSkill") {
        return nullptr;
    }

    return nullptr;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスがアクションノードかどうか
bool BehaviorTreeRunner::IsCompositeNode(
    const NodeInstance &nodeInstance) const {
  return (nodeInstance.typeId == "Selector" ||
          nodeInstance.typeId == "Sequence");
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスがアクションノードかどうか
void BehaviorTreeRunner::RebuildAllPinsMap() {
  m_graphdata.allPins.clear();

  for (auto &node : m_graphdata.nodes) {
    for (auto &pin : node.inputPins) {
      m_graphdata.allPins[pin.id] = pin;
    }
    for (auto &pin : node.outputPins) {
      m_graphdata.allPins[pin.id] = pin;
    }
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ビヘイビアツリーの実行
BehaviorState BehaviorTreeRunner::Execute() {
  if (m_rootNode == nullptr) {
    return BehaviorState::FAILURE;
  }

  if (m_executorContext == nullptr) {
    std::cerr << "ERROR:Executor context is not set for behavior tree execution"
              << std::endl;
    return BehaviorState::FAILURE;
  }

  OutputDebugStringA("DEBUG: BT Runner Execute Start\n");
  return m_rootNode->Execute(m_executorContext);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ルートノードを見つける
void BehaviorTreeRunner::ResetTree() {
  if (m_rootNode) {
    m_rootNode->Initialize();
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// グラフデータからビヘイビアツリーを構築する
bool BehaviorTreeRunner::LoadGraph(const GraphData &graphdata) {
	// グラフデータからビヘイビアツリーを構築する
    m_rootNode = nullptr;
    m_runtimeNodes.clear();
    ownedNodes.clear();
    m_graphdata = graphdata;

    // CSV(NodeValueRegistry)で数値/真偽値プロパティを上書きする。
    // インラインエディタは数値編集を無効化済みのため、実行時の値はCSV由来となる。
    // Enum(targetBox 等)はCSVに無いためグラフ側のプルダウン値が温存される。
    for (auto &node : m_graphdata.nodes) {
        NodeValueRegistry::Instance().ApplyOverrides(node.typeId, node.properties);
    }

	// すべてのピンをIDマップに再構築
    RebuildAllPinsMap();

	// ルートノードを見つける
    const NodeInstance* rootNodeInstance = nullptr;
    int rootCount = 0;
    for (const auto& node : m_graphdata.nodes) {
        if (node.typeId == "Root") {
            rootNodeInstance = &node;
            rootCount++;
        }
    }

	// ルートノードの存在チェック
    if (rootCount == 0) {
        std::cerr << "ERROR: No 'Root' (Start) node found in the graph."
            << std::endl;
        return false;
    }

	// ルートノードの重複チェック
    if (rootCount > 1) {
        std::cerr << "ERROR: Multiple 'Root' (Start) nodes found. Only one is "
            "allowed."
            << std::endl;
        return false;
    }

	// ルートノードから到達可能なノードを探索
    std::set<std::string> reachableNodeIds;
    std::vector<std::string> queue;

	// ルートノードをキューに追加
    if (rootNodeInstance) {
        reachableNodeIds.insert(rootNodeInstance->id);
        queue.push_back(rootNodeInstance->id);
    }

	// キューが空になるまでループ
    size_t head = 0;
    while (head < queue.size()) {
        std::string currentId = queue[head++];

        const NodeInstance* currentNode = FindNodeInctanceById(currentId);
        if (!currentNode)
            continue;

        for (const auto& pin : currentNode->outputPins) {
            for (const auto& link : m_graphdata.links) {
                if (link.startPinId == pin.id) {
                    const PinInstance* targetPin = FindPinInstance(link.endPinId);
                    if (targetPin) {
                        std::string targetNodeId = targetPin->parentNodeId;
                        if (reachableNodeIds.find(targetNodeId) == reachableNodeIds.end()) {
                            reachableNodeIds.insert(targetNodeId);
                            queue.push_back(targetNodeId);
                        }
                    }
                }
            }
        }

		// 入力ピンもチェックして、逆方向の接続も考慮
        for (const auto& pin : currentNode->inputPins) {
            if (pin.IsDataPin() || pin.type == PinType::DataIn_Generic) {
                for (const auto& link : m_graphdata.links) {
                    if (link.endPinId == pin.id) {
                        const PinInstance* sourcePin = FindPinInstance(link.startPinId);
                        if (sourcePin) {
                            std::string sourceNodeId = sourcePin->parentNodeId;
                            if (reachableNodeIds.find(sourceNodeId) ==
                                reachableNodeIds.end()) {
                                reachableNodeIds.insert(sourceNodeId);
                                queue.push_back(sourceNodeId);
                            }
                        }
                    }
                }
            }
        }
    }

	// 到達可能なノードのみを生成してマップに保存
    std::map<std::string, BehaviorTreeNode*> nodeIdToPtr;

	// CSV上書き済みの m_graphdata.nodes を使ってノードを生成する。
    // graphdata（引数）ではなく m_graphdata を参照しないと
    // ApplyOverrides で書いたCSV値がノード生成に届かない。
    for (const auto& nodeInstance : m_graphdata.nodes) {
        if (reachableNodeIds.find(nodeInstance.id) == reachableNodeIds.end()) {
            continue;
        }

        Nodeptr nodeptr = CreateBehaviorNode(nodeInstance);
        if (!nodeptr) {
            continue;
        }

        nodeIdToPtr[nodeInstance.id] = nodeptr.get();
        ownedNodes.push_back(std::move(nodeptr));
    }

	// 階層構造を構築
    if (!BulidHierarchy(nodeIdToPtr)) {
        std::cerr << "ERROR: Failed to build hierarchy." << std::endl;
        return false;
    }

	// ルートノードを見つける
    if (!FindRootNode()) {
        std::cerr << "ERROR: Failed to find root node after building hierarchy."
            << std::endl;
        return false;
    }

    return true;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ビヘイビアツリーの階層構造を構築する
bool BehaviorTreeRunner::BulidHierarchy(
    std::map<std::string, BehaviorTreeNode *> &nodeIdToPtr) {
  std::set<std::string> childNodeIds;

  // すべてのリンクを
  for (const auto &link : m_graphdata.links) {
    const PinInstance *startPin = FindPinInstance(link.startPinId);
    const PinInstance *endPin = FindPinInstance(link.endPinId);

	// デバッグ: リンクの接続情報を出力
    if (!startPin || !endPin) continue;

	// デバッグ: ピンの種類を出力
    bool isStartControl = (startPin->type == PinType::ControlIn ||
                           startPin->type == PinType::ControlOut);
    bool isEndControl = (endPin->type == PinType::ControlIn ||
                         endPin->type == PinType::ControlOut);

	// デバッグ: ピンの種類を出力
    if (!(isStartControl && isEndControl)) continue;

	// デバッグ: 接続情報を出力
    const std::string &parentId = startPin->parentNodeId;
    const std::string &childId = endPin->parentNodeId;

	// デバッグ: 接続情報を出力
    auto parentIt = nodeIdToPtr.find(parentId);
    auto childIt = nodeIdToPtr.find(childId);

	// デバッグ: 接続情報を出力
    if (parentIt == nodeIdToPtr.end() || childIt == nodeIdToPtr.end())
      continue;

	// デバッグ: 接続情報を出力
    const NodeInstance *parentInstance = FindNodeInctanceById(parentId);
    const NodeInstance *childInstance = FindNodeInctanceById(childId);

	// デバッグ: 接続情報を出力
    if (!parentInstance || !childInstance)
      continue;

	// デバッグ: 接続情報を出力
    BehaviorTreeNode *parentNode = parentIt->second;
    BehaviorTreeNode *childNode = childIt->second;

	// デバッグ: 接続情報を出力
    if (IsCompositeNode(*parentInstance) || parentInstance->typeId == "Root" ||
        parentInstance->typeId == "Inverter") {
		// コンポジットノードは子ノードとして接続
        for (auto it = ownedNodes.begin(); it != ownedNodes.end(); ++it) {
            if (it->get() == childNode) {
                parentNode->AddChild(std::move(*it));
                ownedNodes.erase(it);
                childNodeIds.insert(childId);
                std::cout << "DEBUG: Added child " << childId << " to composite "
                    << parentId << std::endl;
                break;
            }
        }      
    }
	// アクションノードは次のノードとして接続
    else if (IsActionNode(*parentInstance)) {
        // デバッグ: アクションノードの接続を出力
        bool linkFound = false;

        // アクションノードは次のノードとして接続
        for (auto it = ownedNodes.begin(); it != ownedNodes.end(); ++it) {
            if (it->get() == childNode) {
                parentNode->SetNextNode(std::move(*it));
                ownedNodes.erase(it);
                childNodeIds.insert(childId);
                linkFound = true;

                std::cout << "DEBUG: Set next node " << childId << " for action "
                    << parentId << std::endl;

                break;
            }
        }

		// デバッグ: アクションノードの接続が見つからない場合はエラーを出力
        if (!linkFound) {
            std::cout << "ERROR: Failed to link Action " << parentId << " -> "
                << childId << ". Child node not found in ownedNodes!"
                << std::endl;
        }
    }
	// 条件ノードはTrue/Falseブランチとして接続
    else if (IsConditionNode(*parentInstance)) {
        bool isTrueBranch = (startPin->name == "True");
        bool isFalseBranch = (startPin->name == "False");

		// デバッグ: 条件ノードのブランチ情報を出力
        for (auto it = ownedNodes.begin(); it != ownedNodes.end(); ++it) {
			// デバッグ: 条件ノードのブランチ情報を出力
            if (it->get() == childNode) {
				// 条件ノードのTrue/Falseブランチに接続
                if (auto* rangeNode =
                    dynamic_cast<CheckAttackRangeCondition*>(parentNode)) {
                    if (isTrueBranch) {
                        rangeNode->SetTrueNode(std::move(*it));
                    }
                    else if (isFalseBranch) {
                        rangeNode->SetFalseNode(std::move(*it));
                    }
                }
				// 条件分岐ノードのTrue/Falseブランチに接続
                else if (auto* condNode =
                    dynamic_cast<ConditionNode*>(parentNode)) {
                    if (isTrueBranch) {
                        condNode->SetTrueNode(std::move(*it));
                    }
                    else if (isFalseBranch) {
                        condNode->SetFalseNode(std::move(*it));
                    }
                }

				// デバッグ: 条件ノードのブランチ情報を出力
                if (isTrueBranch || isFalseBranch) {
                    std::cout << "DEBUG: Set branch node " << childId
                        << " for condition " << parentId
                        << " (True=" << isTrueBranch << ")" << std::endl;
                }

				// デバッグ: 条件ノードのブランチ情報を出力
                ownedNodes.erase(it);
                childNodeIds.insert(childId);
                break;
            }
        }
    }
  }

  return true;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスがアクションノードかどうか
bool BehaviorTreeRunner::IsActionNode(const NodeInstance &nodeInstance) const {
  return !IsCompositeNode(nodeInstance) && !IsConditionNode(nodeInstance) &&
         nodeInstance.typeId != "Root" && nodeInstance.typeId != "Inverter";
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ルートノードを見つける
bool BehaviorTreeRunner::FindRootNode() {
    // ルートノードを見つける
    for (const auto& node : ownedNodes) {
        const NodeInstance* instance = FindNodeInctanceById(node->GetId().id);
        if (instance && instance->typeId == "Root") {
            m_rootNode = node.get();
            std::cout << "INFO: Root node found: " << instance->displayName
                << std::endl;

            if (ownedNodes.size() > 1) {
                std::cout << "WARNING: Multiple top-level nodes found (disconnected "
                    "nodes). Using 'Root' node."
                    << std::endl;
            }
            return true;
        }
    }

	// ルートノードが見つからない場合、トップレベルのノードが1つだけ存在すればそれをルートとする
    if (ownedNodes.size() == 1) {
        m_rootNode = ownedNodes[0].get();
        const NodeInstance* rootInstance =
            FindNodeInctanceById(m_rootNode->GetId().id);
        const std::string& rootName =
            rootInstance ? rootInstance->displayName : "Unknown Node";

        std::cout << "INFO: Single root node found (fallback): " << rootName
            << std::endl;
        return true;
    }

    std::cerr << "ERROR: Failed to determine root node. Multiple top-level nodes "
        "exist but none are of type 'Root'."
        << std::endl;
    return false;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスが条件ノードかどうか
bool BehaviorTreeRunner::IsConditionNode(
    const NodeInstance &nodeInstance) const {
  return (nodeInstance.typeId == "Condition_IsTargetInAttackRange" ||
          nodeInstance.typeId == "Condition_If");
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// あるノードの入力ピンに接続されている出力ピンの親ノードIDを取得
std::string
BehaviorTreeRunner::GetInputSourceNodeId(const std::string &nodeId,
                                         const std::string &pinName) const {
	// あるノードの入力ピンに接続されている出力ピンの親ノードIDを取得
    const NodeInstance* targetNode = nullptr;
    for (const auto& node : m_graphdata.nodes) {
        if (node.id == nodeId) {
            targetNode = &node;
            break;
        }
    }

	// ターゲットノードが見つからない場合は空文字を返す
    if (!targetNode)
        return "";

	// ターゲットノードの指定された入力ピンを見つける
    const PinInstance* targetPin = nullptr;
    for (const auto& pin : targetNode->inputPins) {
        if (pin.name == pinName) {
            targetPin = &pin;
            break;
        }
    }

	// ターゲットピンが見つからない場合は空文字を返す
    if (!targetPin)
        return "";

	// ターゲットピンに接続されているリンクを探し、接続元のノードIDを返す
    for (const auto& link : m_graphdata.links) {
        if (link.endPinId == targetPin->id) {
            const auto it = m_graphdata.allPins.find(link.startPinId);
            if (it != m_graphdata.allPins.end()) {
                return it->second.parentNodeId;
            }
        }
    }

    return "";
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードIDからNodeInstanceを取得
const NodeInstance *
BehaviorTreeRunner::GetNodeInstance(const std::string &nodeId) const {
    return FindNodeInctanceById(nodeId);
}
//-----------------------------------------------------------------------------