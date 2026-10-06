#include "NodeGraph.h"
#include "CDrawUtil.h"
#include "DInput.h"
#include "GameMain.h"
#include "NodeDefinition.h"
#include <algorithm>
#include <iostream>
#include <set>

namespace {
    constexpr float DEFAULT_NODE_WIDTH = 180.0f;
    constexpr float NODE_HEADER_HEIGHT = 30.0f;
    constexpr float PIN_RADIUS = 5.0f;
    constexpr float PIN_SPACING = 30.0f;
    constexpr float NODE_PADDING = 10.0f;

    //-----------------------------------------------------------------------------
    // ピンタイプからベースタイプ（互換性チェック用）を取得
    int GetPinBaseType(PinType t) {
        switch (t) {
        case PinType::ControlIn:
        case PinType::ControlOut:
            return 1;
        case PinType::DataIn:
        case PinType::DataOut:
            return 2;
        case PinType::DataIn_Int:
        case PinType::DataOut_Int:
            return 3;
        case PinType::DataIn_Float:
        case PinType::DataOut_Float:
            return 4;
        case PinType::DataIn_String:
        case PinType::DataOut_String:
            return 5;
        case PinType::DataIn_Bool:
        case PinType::DataOut_Bool:
            return 6;
        case PinType::DataIn_Enum:
        case PinType::DataOut_Enum:
            return 7;
        case PinType::DataIn_Vector3:
        case PinType::DataOut_Vector3:
            return 8;
        case PinType::DataIn_Generic:
        case PinType::DataOut_Generic:
            return 9;
        case PinType::DataIn_Skill:
        case PinType::DataOut_Skill:
            return 10;
        default:
            return 0;
        }
    }
    //-----------------------------------------------------------------------------

    //-----------------------------------------------------------------------------
    // ピン同士の互換性をチェック
    bool CheckPinCompatibility(const PinInstance* p1, const PinInstance* p2) {
        if (!p1 || !p2)
            return false;
        if (p1 == p2)
            return false;
        if (p1->parentNodeId == p2->parentNodeId)
            return false;
        if (p1->IsInput() == p2->IsInput())
            return false;

        int type1 = GetPinBaseType(p1->type);
        int type2 = GetPinBaseType(p2->type);

        return type1 == type2;
    }
} // namespace
//-----------------------------------------------------------------------------

constexpr float PIN_HIT_RADIUS = 7.0f;

//-----------------------------------------------------------------------------
// ノードグラフのコンストラクタ
NodeGraph::NodeGraph(std::shared_ptr<NodeFactory> factory)
    : nodeFactory_(std::move(factory)) {
    UpdateAllPinsMap();

    wasLeftClickDown_ = false;
    wasRightClickDown_ = false;
    isDraggingNewNode_ = false;
    showPropertyPanel_ = false;

    cameraOffset_ = VECTOR2{ 0.0f, 0.0f };
    lastMousePos_ = VECTOR2{ 0.0f, 0.0f };
    propertyPanelPos_ = VECTOR2{ 0.0f, 0.0f };

    selectedNode_ = nullptr;
    draggingNode_ = nullptr;
    draggingStartPin_ = nullptr;
    propertyPanelNode_ = nullptr;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// グラフデータの設定と初期化
void NodeGraph::SetGraphData(GraphData&& data) {
    graphData_ = std::move(data);

    // ロード後にテンプレートからpropertyDefinitionsを復元
    RestorePropertyDefinitionsFromTemplates();

    UpdateAllPinsMap();
    PropagateDataLinkValues();
    std::cout << "INFO: NodeGraph data updated. Total nodes: "
        << graphData_.nodes.size() << std::endl;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// IDからピンインスタンスを取得
PinInstance* NodeGraph::GetPinById(const std::string& pinId) {
    for (auto& node : graphData_.nodes) {
        for (auto& pin : node.inputPins) {
            if (pin.id == pinId)
                return &pin;
        }
        for (auto& pin : node.outputPins) {
            if (pin.id == pinId)
                return &pin;
        }
    }
    return nullptr;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ルートノードが存在するか確認
bool NodeGraph::HasRootNode() const {
    for (const auto& node : graphData_.nodes) {
        if (node.typeId == "Root") {
            return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 新規ノードの配置とドラッグ開始
void NodeGraph::StartNodeDrag(const std::string& templateName,
    const VECTOR2& screenPosition) {
    if (isDraggingNewNode_) {
        return;
    }

    VECTOR2 spawnScreenPos = { screenPosition.x - (DEFAULT_NODE_WIDTH / 2.0f),
                              screenPosition.y - (NODE_HEADER_HEIGHT / 2.0f) };

    AddNode(templateName, spawnScreenPos);

    if (!graphData_.nodes.empty()) {
        NodeInstance* newNode = &graphData_.nodes.back();

        // Adjust position based on actual width to center it
        float actualWidth = newNode->GetWidth();
        newNode->position.x =
            screenPosition.x + cameraOffset_.x - (actualWidth / 2.0f);

        ClearNodeSelection();

        selectedNode_ = newNode;
        selectedNode_->isSelected = true;
        draggingNode_ = newNode;
        isDraggingNewNode_ = true;

        lastMousePos_ = screenPosition;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 選択中のノードを削除
void NodeGraph::DeleteSelectedNode() {
    if (!selectedNode_) {
        return;
    }

    const std::string nodeId = selectedNode_->id;
    DeleteNodeById(nodeId);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 全ノードの削除
void NodeGraph::DeleteAllNodes() {
    graphData_.nodes.clear();
    graphData_.links.clear();
    graphData_.allPins.clear();

    graphData_.nextNodeId = 0;
    graphData_.nextLinkId = 0;

    selectedNode_ = nullptr;
    draggingNode_ = nullptr;
    draggingStartPin_ = nullptr;
    isDraggingNewNode_ = false;
    wasLeftClickDown_ = false;
    showPropertyPanel_ = false;

    canvasMousePos_ = VECTOR2{ 0.0f, 0.0f };
    lastMousePos_ = VECTOR2{ 0.0f, 0.0f };
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ピンIDマップの更新
void NodeGraph::UpdateAllPinsMap() {
    graphData_.allPins.clear();
    for (const auto& node : graphData_.nodes) {
        for (const auto& pin : node.inputPins) {
            graphData_.allPins[pin.id] = pin;
        }
        for (const auto& pin : node.outputPins) {
            graphData_.allPins[pin.id] = pin;
        }
    }
    std::cout << "DEBUG: All pins map updated. Total pins: "
        << graphData_.allPins.size() << std::endl;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノード選択の解除
void NodeGraph::ClearNodeSelection() {
    if (selectedNode_) {
        selectedNode_->isSelected = false;
    }
    selectedNode_ = nullptr;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 指定IDのノードと関連リンクを削除
void NodeGraph::DeleteNodeById(const std::string& nodeId) {
    auto nodeIt =
        std::find_if(graphData_.nodes.begin(), graphData_.nodes.end(),
            [&](const NodeInstance& n) { return n.id == nodeId; });

    if (nodeIt == graphData_.nodes.end()) {
        return;
    }

    std::set<std::string> pinIds;
    for (const auto& p : nodeIt->inputPins)
        pinIds.insert(p.id);
    for (const auto& p : nodeIt->outputPins)
        pinIds.insert(p.id);

    auto removeIt = std::remove_if(
        graphData_.links.begin(), graphData_.links.end(),
        [&](const LinkInstance& link) {
            bool touchStart = pinIds.count(link.startPinId) > 0;
            bool touchEnd = pinIds.count(link.endPinId) > 0;

            if (!touchStart && !touchEnd) {
                return false;
            }
            auto removeFromOtherPin = [&](const std::string& otherPinId,
                const std::string& thisPinId) {
                    if (PinInstance* other = GetPinById(otherPinId)) {
                        auto& vec = other->linkedPinIds;
                        vec.erase(std::remove(vec.begin(), vec.end(), thisPinId),
                            vec.end());
                    }
                };

            if (!touchStart) {
                removeFromOtherPin(link.startPinId, link.endPinId);
            }
            if (!touchEnd) {
                removeFromOtherPin(link.endPinId, link.startPinId);
            }

            return true;
        });

    graphData_.links.erase(removeIt, graphData_.links.end());

    for (const auto& pid : pinIds) {
        graphData_.allPins.erase(pid);
    }

    if (draggingNode_ && draggingNode_->id == nodeId) {
        draggingNode_ = nullptr;
    }
    if (selectedNode_ && selectedNode_->id == nodeId) {
        selectedNode_ = nullptr;
    }
    if (draggingStartPin_ && pinIds.count(draggingStartPin_->id) > 0) {
        draggingStartPin_ = nullptr;
    }
    isDraggingNewNode_ = false;

    graphData_.nodes.erase(nodeIt);

    UpdateAllPinsMap();

    if (onGraphChanged_)
        onGraphChanged_();

    std::cout << "INFO: Node deleted: " << nodeId << std::endl;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// データリンクの値を伝播させる
void NodeGraph::PropagateDataLinkValues() {
    for (const auto& link : graphData_.links) {
        PinInstance* startPin = GetPinById(link.startPinId);
        PinInstance* endPin = GetPinById(link.endPinId);

        if (!startPin || !endPin)
            continue;
        if (!startPin->IsDataPin())
            continue;

        std::string value;

        for (const auto& node : graphData_.nodes) {
            for (const auto& pin : node.outputPins) {
                if (pin.id == startPin->id) {
                    auto it = node.properties.find("value");
                    if (it != node.properties.end()) {
                        value = it->second;
                    }
                    break;
                }
            }
        }

        endPin->dataValue = value;

        if (!endPin->linkedPropertyName.empty()) {
            for (auto& node : graphData_.nodes) {
                for (auto& pin : node.inputPins) {
                    if (node.id == endPin->parentNodeId) {
                        node.properties[endPin->linkedPropertyName] = value;
                        break;
                    }
                }
            }
        }
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// テンプレートからプロパティ定義を復元する
void NodeGraph::RestorePropertyDefinitionsFromTemplates() {
    const auto& templates = nodeFactory_->GetAllTemplates();

    for (auto& node : graphData_.nodes) {
        auto tmplIt = templates.find(node.typeId);
        if (tmplIt == templates.end()) {
            std::cerr << "WARNING: Template not found for node type: " << node.typeId
                << std::endl;
            continue;
        }

        const NodeTemplate& tmpl = tmplIt->second;

        node.propertyDefinitions.clear();
        for (const auto& propDef : tmpl.propertyDefinitions) {
            node.propertyDefinitions[propDef.name] = propDef;
        }

        for (size_t i = 0; i < node.inputPins.size() && i < tmpl.inputs.size();
            ++i) {
            node.inputPins[i].type = tmpl.inputs[i].type; // Sync Type
            if (node.inputPins[i].linkedPropertyName.empty()) {
                node.inputPins[i].linkedPropertyName =
                    tmpl.inputs[i].linkedPropertyName;
            }
        }

        for (size_t i = 0; i < node.outputPins.size() && i < tmpl.outputs.size();
            ++i) {
            node.outputPins[i].type = tmpl.outputs[i].type; // Sync Type
            if (node.outputPins[i].linkedPropertyName.empty()) {
                node.outputPins[i].linkedPropertyName =
                    tmpl.outputs[i].linkedPropertyName;
            }
        }
    }

    std::cout << "INFO: Restored property definitions from templates for "
        << graphData_.nodes.size() << " nodes." << std::endl;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// インライン入力フィールドの登録と管理
void NodeGraph::RegisterInlineInputFields() {
    lineManager_.BeginFrame();

    if (graphData_.nodes.empty()) {
        lineManager_.EndFrame();
        return;
    }

    for (auto& node : graphData_.nodes) {
        VECTOR2 nodeScreenPos = { (node.position.x - cameraOffset_.x) * zoomScale_,
                                 (node.position.y - cameraOffset_.y) * zoomScale_ };

        float pinYOffset =
            (NODE_HEADER_HEIGHT + NODE_PADDING + PIN_SPACING / 2.0f) * zoomScale_;

        for (auto& pin : node.inputPins) {
            if (!pin.IsDataPin()) {
                pinYOffset += PIN_SPACING * zoomScale_;
                continue;
            }

            if (pin.IsConnected()) {
                pinYOffset += PIN_SPACING * zoomScale_;
                continue;
            }

            std::string propName = pin.linkedPropertyName;
            if (propName.empty()) {
                pinYOffset += PIN_SPACING * zoomScale_;
                continue;
            }

            // Prop retrieval
            auto propIt = node.properties.find(propName);
            if (propIt == node.properties.end()) {
                std::string defaultValue = "0";
                auto defIt = node.propertyDefinitions.find(propName);
                if (defIt != node.propertyDefinitions.end()) {
                    defaultValue = defIt->second.defaultValue;
                }
                node.properties[propName] = defaultValue;
                propIt = node.properties.find(propName);
            }

            // Create Field
            InlineInputField field;
            field.id = node.id + "_" + pin.id;
            field.valuePtr = &propIt->second;

            // Position Calc
            float pinNameWidth =
                ImGui::CalcTextSize(MyImgui::GetLocalizedText(pin.name).c_str()).x;
            float scaledPinRadius = PIN_RADIUS * zoomScale_;

            field.position.x = nodeScreenPos.x + scaledPinRadius +
                (8.0f * zoomScale_) + pinNameWidth +
                (8.0f * zoomScale_);
            field.position.y = nodeScreenPos.y + pinYOffset - (12.0f * zoomScale_);

            float nodeWidth = node.GetWidth() * zoomScale_;
            float availableWidth = nodeWidth - (field.position.x - nodeScreenPos.x) -
                (10.0f * zoomScale_);

            field.width =
                std::max<float>(40.0f * zoomScale_,
                    std::min<float>(100.0f * zoomScale_, availableWidth));
            field.height = 24.0f * zoomScale_;

            auto defIt = node.propertyDefinitions.find(propName);
            if (defIt != node.propertyDefinitions.end()) {
                const PropertyDefinition& propDef = defIt->second;

                // 数値/真偽値プロパティはCSV(NodeValueRegistry)で管理するため
                // インライン編集を無効化する。Enum(プルダウン)のみ編集可能。
                if (propDef.type == PropertyType::Float ||
                    propDef.type == PropertyType::Int ||
                    propDef.type == PropertyType::Bool) {
                    pinYOffset += PIN_SPACING * zoomScale_;
                    continue;
                }

                switch (propDef.type) {
                case PropertyType::Float:
                    field.type = InlineInputType::Float;
                    field.width = 80.0f * zoomScale_;
                    field.minValue = propDef.minValue;
                    field.maxValue = propDef.maxValue;
                    field.dragSpeed = (propDef.maxValue - propDef.minValue) / 100.0f;
                    break;
                case PropertyType::Int:
                    field.type = InlineInputType::Int;
                    field.width = 80.0f * zoomScale_;
                    field.minValue = propDef.minValue;
                    field.maxValue = propDef.maxValue;
                    field.dragSpeed = 1.0f;
                    break;
                case PropertyType::Bool:
                    field.type = InlineInputType::Bool;
                    field.width = 24.0f * zoomScale_; // Checkbox size
                    break;
                case PropertyType::Enum:
                    field.type = InlineInputType::Enum;
                    {
                        const EnumDefinition* enumDef =
                            EnumRegistry::GetInstance().GetEnum(propDef.enumTypeName);
                        if (enumDef) {
                            field.enumValues = enumDef->values;
                            float maxEnumW = 0.0f;
                            for (const auto& val : enumDef->values) {
                                float ew = ImGui::CalcTextSize(val.c_str()).x;
                                if (ew > maxEnumW)
                                    maxEnumW = ew;
                            }
                            field.width = (maxEnumW + 30.0f) * zoomScale_;
                        }
                    }
                    break;
                default:
                    field.type = InlineInputType::String;
                    break;
                }
            }
            else {
                field.type = InlineInputType::String;
            }

            lineManager_.RegisterField(field);

            pinYOffset += PIN_SPACING * zoomScale_;
        }

        // 入力ピンを持たないノード（Variableノードなど）のための特別処理
        // プロパティ定義があり、入力ピンがない場合、インラインフィールドを表示する
        if (node.inputPins.empty() && !node.propertyDefinitions.empty()) {
            for (auto& pair : node.propertyDefinitions) {
                if (!pair.second.isVisible)
                    continue;
                const auto& propName = pair.first;
                const auto& propDef = pair.second;

                // 数値/真偽値プロパティはCSV(NodeValueRegistry)で管理するため
                // インライン編集を無効化する。Enum(プルダウン)のみ編集可能。
                if (propDef.type == PropertyType::Float ||
                    propDef.type == PropertyType::Int ||
                    propDef.type == PropertyType::Bool) {
                    continue;
                }

                // プロパティの値を取得（存在しない場合はデフォルト値を設定）
                if (node.properties.find(propName) == node.properties.end()) {
                    node.properties[propName] = propDef.defaultValue;
                }
                auto& propValue = node.properties[propName];

                // Create Field
                InlineInputField field;
                field.id = node.id + "_Inline_" + propName;
                field.valuePtr = &propValue;

                // Position Calculation (Force left alignment for input-less nodes)
                field.position.x = nodeScreenPos.x + (15.0f * zoomScale_);
                field.position.y = nodeScreenPos.y + pinYOffset - (12.0f * zoomScale_);

                // Ensure field fits within node or extends reasonably
                field.width = 120.0f * zoomScale_;
                field.height = 24.0f * zoomScale_;

                switch (propDef.type) {
                case PropertyType::Float:
                    field.type = InlineInputType::Float;
                    field.width = 80.0f * zoomScale_;
                    field.minValue = propDef.minValue;
                    field.maxValue = propDef.maxValue;
                    field.dragSpeed = (propDef.maxValue - propDef.minValue) / 100.0f;
                    if (field.dragSpeed <= 0.0001f)
                        field.dragSpeed = 0.1f;
                    break;
                case PropertyType::Int:
                    field.type = InlineInputType::Int;
                    field.width = 80.0f * zoomScale_;
                    field.minValue = propDef.minValue;
                    field.maxValue = propDef.maxValue;
                    field.dragSpeed = 1.0f;
                    break;
                case PropertyType::Bool:
                    field.type = InlineInputType::Bool;
                    field.width = 24.0f * zoomScale_; // Checkbox size
                    break;
                case PropertyType::Enum:
                    field.type = InlineInputType::Enum;
                    {
                        const EnumDefinition* enumDef =
                            EnumRegistry::GetInstance().GetEnum(propDef.enumTypeName);
                        if (enumDef) {
                            field.enumValues = enumDef->values;
                            float maxEnumW = 0.0f;
                            for (const auto& val : enumDef->values) {
                                float ew = ImGui::CalcTextSize(val.c_str()).x;
                                if (ew > maxEnumW)
                                    maxEnumW = ew;
                            }
                            field.width = (maxEnumW + 30.0f) * zoomScale_;
                        }
                        else {
                            field.enumValues.push_back("NoEnum:" + propDef.enumTypeName);
                        }
                    }
                    break;
                default:
                    field.type = InlineInputType::String;
                    break;
                }

                lineManager_.RegisterField(field);

                pinYOffset += PIN_SPACING * zoomScale_;
            }
        }
    }
    lineManager_.EndFrame();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードをグラフに追加
void NodeGraph::AddNode(const std::string& templateName,
    const VECTOR2& screenPosition) {
    if (templateName == "Root" && HasRootNode()) {
        std::cout << "WARNING: Root node already exists. Cannot add another one."
            << std::endl;
        return;
    }

    VECTOR2 canvasPos = { screenPosition.x + cameraOffset_.x,
                         screenPosition.y + cameraOffset_.y };

    auto newNode =
        nodeFactory_->CreateNode(templateName, canvasPos, graphData_.nextNodeId);

    if (newNode) {
        graphData_.nodes.push_back(*newNode);
        graphData_.nextNodeId++;

        RestorePropertyDefinitionsFromTemplates();
        UpdateAllPinsMap();
        if (onGraphChanged_)
            onGraphChanged_();
        std::cout << "INFO: Node added: " << newNode->id << std::endl;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ピン間にリンクを作成
void NodeGraph::CreateLink(const std::string& startPinId,
    const std::string& endPinId) {
    PinInstance* startPin = GetPinById(startPinId);
    PinInstance* endPin = GetPinById(endPinId);

    if (!startPin || !endPin) {
        std::cerr << "ERROR: Cannot create link. One or both pins not found."
            << std::endl;
        return;
    }

    LinkInstance newLink;
    newLink.id = "Link_" + std::to_string(graphData_.nextLinkId++);

    if (startPin->IsInput()) {
        newLink.startPinId = endPinId;
        newLink.endPinId = startPinId;
        std::swap(startPin, endPin);
    }
    else {
        newLink.startPinId = startPinId;
        newLink.endPinId = endPinId;
    }

    startPin->linkedPinIds.push_back(newLink.endPinId);
    endPin->linkedPinIds.push_back(newLink.startPinId);

    graphData_.links.push_back(newLink);

    if (startPin->IsDataPin()) {
        PropagateDataLinkValues();
    }

    if (onGraphChanged_)
        onGraphChanged_();

    std::cout << "INFO: Link created: " << newLink.id << std::endl;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// スクリーン座標をキャンバス座標に変換
VECTOR2 NodeGraph::ScreenToCanvas(const VECTOR2& screenPos) const {
    return VECTOR2{ (screenPos.x / zoomScale_) + cameraOffset_.x,
                   (screenPos.y / zoomScale_) + cameraOffset_.y };
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// キャンバス座標をスクリーン座標に変換
VECTOR2 NodeGraph::CanvasToScreen(const VECTOR2& canvasPos) const {
    return VECTOR2{ (canvasPos.x - cameraOffset_.x) * zoomScale_,
                   (canvasPos.y - cameraOffset_.y) * zoomScale_ };
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 入力イベントのハンドリング（操作全般）
void NodeGraph::HandleInput(const VECTOR2& mousePos, bool isLeftClickDown,
    bool isRightClickDown) {
    //
    VECTOR2 screenMousePos = mousePos;

    long z = GameDevice()->m_pDI->GetMouseState().lZ;
    if (z != 0) {
        float oldZoom = zoomScale_;
        if (z > 0)
            zoomScale_ += 0.1f;
        else
            zoomScale_ -= 0.1f;

        if (zoomScale_ < 0.7f)
            zoomScale_ = 0.7f;
        if (zoomScale_ > 1.0f)
            zoomScale_ = 1.0f;

        VECTOR2 mouseCanvasPos = { (screenMousePos.x / oldZoom) + cameraOffset_.x,
                                  (screenMousePos.y / oldZoom) + cameraOffset_.y };

        cameraOffset_.x = mouseCanvasPos.x - (screenMousePos.x / zoomScale_);
        cameraOffset_.y = mouseCanvasPos.y - (screenMousePos.y / zoomScale_);
    }

    canvasMousePos_ = ScreenToCanvas(mousePos);

    bool leftPressedThisFrame = (isLeftClickDown && !wasLeftClickDown_);
    bool leftReleasedThisFrame = (!isLeftClickDown && wasLeftClickDown_);
    bool rightPressedThisFrame = (isRightClickDown && !wasRightClickDown_);

    if (rightPressedThisFrame) {
        isPanning_ = true;
        panStartMousePos_ = screenMousePos;
        panStartCameraOffset_ = cameraOffset_;
    }

    if (isRightClickDown && isPanning_) {
        float dx = screenMousePos.x - panStartMousePos_.x;
        float dy = screenMousePos.y - panStartMousePos_.y;

        cameraOffset_.x = panStartCameraOffset_.x - (dx / zoomScale_);
        cameraOffset_.y = panStartCameraOffset_.y - (dy / zoomScale_);
    }

    lineManager_.ProcessKeyboardInput();

    bool isLeftClickUp = (!isLeftClickDown && wasLeftClickDown_);

    bool captured =
        lineManager_.ProcessInput(screenMousePos, isLeftClickDown, isLeftClickUp);

    if (lineManager_.WasValueChanged()) {
        PropagateDataLinkValues();
    }
    if (lineManager_.WasEditFinished()) {
        if (onGraphChanged_)
            onGraphChanged_();
    }

    if (captured) {
        wasLeftClickDown_ = isLeftClickDown;
        wasRightClickDown_ = isRightClickDown;
        return;
    }

    if (!isRightClickDown && wasRightClickDown_) {
        float dx = screenMousePos.x - panStartMousePos_.x;
        float dy = screenMousePos.y - panStartMousePos_.y;
        float dragDistSq = dx * dx + dy * dy;

        isPanning_ = false;

        if (dragDistSq < 100.0f) {
            NodeInstance* hitNode = HitTestNode(canvasMousePos_);
            if (hitNode) {
                showPropertyPanel_ = true;
                propertyPanelPos_ = screenMousePos;
                propertyPanelNode_ = hitNode;
            }
            else {
                showPropertyPanel_ = false;
                propertyPanelNode_ = nullptr;
            }
        }
    }

    if (leftPressedThisFrame && showPropertyPanel_) {
        float panelWidth = 220.0f;
        float panelHeight = 200.0f;

        bool insidePanel = (screenMousePos.x >= propertyPanelPos_.x &&
            screenMousePos.x <= propertyPanelPos_.x + panelWidth &&
            screenMousePos.y >= propertyPanelPos_.y &&
            screenMousePos.y <= propertyPanelPos_.y + panelHeight);

        if (!insidePanel) {
            showPropertyPanel_ = false;
            propertyPanelNode_ = nullptr;
        }
    }

    if (isDraggingNewNode_ && draggingNode_) {
        if (isLeftClickDown) {
            float dx = (screenMousePos.x - lastMousePos_.x) / zoomScale_;
            float dy = (screenMousePos.y - lastMousePos_.y) / zoomScale_;

            draggingNode_->position.x += dx;
            draggingNode_->position.y += dy;

            lastMousePos_ = screenMousePos;
        }
        else {
            isDraggingNewNode_ = false;
            draggingNode_ = nullptr;
            if (onGraphChanged_)
                onGraphChanged_();
            std::cout << "INFO: New node drag finished." << std::endl;
        }
        return;
    }

    if (leftPressedThisFrame) {
        if (PinInstance* hitPin = HitTestPin(canvasMousePos_)) {
            bool isAltDown = (GetKeyState(VK_MENU) & 0x8000) != 0;
            if (isAltDown) {
                std::cout << "INFO: Disconnecting pin: " << hitPin->id << std::endl;

                auto removeIt = std::remove_if(
                    graphData_.links.begin(), graphData_.links.end(),
                    [&](const LinkInstance& link) {
                        bool isConnected = (link.startPinId == hitPin->id ||
                            link.endPinId == hitPin->id);
                        if (isConnected) {
                            std::string otherPinId = (link.startPinId == hitPin->id)
                                ? link.endPinId
                                : link.startPinId;
                            if (PinInstance* other = GetPinById(otherPinId)) {
                                auto& vec = other->linkedPinIds;
                                vec.erase(std::remove(vec.begin(), vec.end(), hitPin->id),
                                    vec.end());
                            }
                        }
                        return isConnected;
                    });

                bool changed = (removeIt != graphData_.links.end());
                graphData_.links.erase(removeIt, graphData_.links.end());

                if (changed) {
                    hitPin->linkedPinIds.clear();
                    if (hitPin->IsDataPin()) {
                        if (hitPin->IsInput()) {
                            for (auto& node : graphData_.nodes) {
                                if (node.id == hitPin->parentNodeId) {
                                    if (!hitPin->linkedPropertyName.empty()) {
                                        auto defIt = node.propertyDefinitions.find(
                                            hitPin->linkedPropertyName);
                                        if (defIt != node.propertyDefinitions.end()) {
                                            node.properties[hitPin->linkedPropertyName] =
                                                defIt->second.defaultValue;
                                        }
                                        else {
                                            node.properties[hitPin->linkedPropertyName] = "";
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    if (onGraphChanged_)
                        onGraphChanged_();
                    std::cout << "INFO: Pin disconnected." << std::endl;
                }

                wasLeftClickDown_ = isLeftClickDown;
                wasRightClickDown_ = isRightClickDown;
                return;
            }

            draggingStartPin_ = hitPin;
            ClearNodeSelection();
            wasLeftClickDown_ = isLeftClickDown;
            wasRightClickDown_ = isRightClickDown;
            return;
        }

        if (NodeInstance* hitNode = HitTestNode(canvasMousePos_)) {
            ClearNodeSelection();
            selectedNode_ = hitNode;
            selectedNode_->isSelected = true;

            draggingNode_ = hitNode;
            lastMousePos_ = screenMousePos;

            wasLeftClickDown_ = isLeftClickDown;
            wasRightClickDown_ = isRightClickDown;
            return;
        }

        ClearNodeSelection();
    }

    if (isLeftClickDown) {
        if (draggingNode_) {
            float dx = (screenMousePos.x - lastMousePos_.x) / zoomScale_;
            float dy = (screenMousePos.y - lastMousePos_.y) / zoomScale_;

            draggingNode_->position.x += dx;
            draggingNode_->position.y += dy;

            lastMousePos_ = screenMousePos;
        }

        if (draggingStartPin_) {
            lastMousePos_ = screenMousePos;
        }
    }

    if (leftReleasedThisFrame) {
        if (draggingStartPin_) {
            PinInstance* hitPin = HitTestPin(canvasMousePos_);
            if (hitPin && hitPin != draggingStartPin_) {
                if (CheckPinCompatibility(draggingStartPin_, hitPin)) {
                    CreateLink(draggingStartPin_->id, hitPin->id);
                }
                else {
                    std::cout << "DEBUG: Incompatible pins." << std::endl;
                }
            }
            draggingStartPin_ = nullptr;
        }

        if (draggingNode_) {
            if (onGraphChanged_)
                onGraphChanged_();
        }
        draggingNode_ = nullptr;
        isDraggingNewNode_ = false;
    }

    wasLeftClickDown_ = isLeftClickDown;
    wasRightClickDown_ = isRightClickDown;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// グラフ全体の描画
void NodeGraph::Draw() {
    for (const auto& link : graphData_.links) {
        DrawLink(link);
    }

    for (const auto& node : graphData_.nodes) {
        node.Draw(cameraOffset_, zoomScale_);
    }

    if (draggingStartPin_) {
        DrawDraggingLink(canvasMousePos_);
    }

    RegisterInlineInputFields();
    lineManager_.Draw(zoomScale_);

    if (showPropertyPanel_) {
        DrawPropertyEditors();
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// リンクの描画
void NodeGraph::DrawLink(const LinkInstance& link) {
    PinInstance* startPin = GetPinById(link.startPinId);
    PinInstance* endPin = GetPinById(link.endPinId);

    if (startPin && endPin) {
        VECTOR2 startPinWorldPos = GetPinWorldPos(startPin);
        VECTOR2 endPinWorldPos = GetPinWorldPos(endPin);

        VECTOR2 startPos = { (startPinWorldPos.x - cameraOffset_.x) * zoomScale_,
                            (startPinWorldPos.y - cameraOffset_.y) * zoomScale_ };
        VECTOR2 endPos = { (endPinWorldPos.x - cameraOffset_.x) * zoomScale_,
                          (endPinWorldPos.y - cameraOffset_.y) * zoomScale_ };

        bool startIsInput = startPin->IsInput();
        bool endIsInput = endPin->IsInput();

        int linkColor = startPin->IsDataPin() ? 0xFF00FFFF : CDrawUtil::COLOR_LINK;

        CDrawUtil::DrawBezierCurve(startPos, endPos, startIsInput, endIsInput,
            linkColor);
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ドラッグ中のリンクの描画
void NodeGraph::DrawDraggingLink(const VECTOR2& mousePos) {
    if (!draggingStartPin_)
        return;

    VECTOR2 startPinWorldPos = GetPinWorldPos(draggingStartPin_);

    VECTOR2 startPos = { (startPinWorldPos.x - cameraOffset_.x) * zoomScale_,
                        (startPinWorldPos.y - cameraOffset_.y) * zoomScale_ };

    VECTOR2 endPos = { (mousePos.x - cameraOffset_.x) * zoomScale_,
                      (mousePos.y - cameraOffset_.y) * zoomScale_ };

    bool startIsInput = draggingStartPin_->IsInput();
    bool endIsInput = !startIsInput;

    unsigned int linkColor = 0XFF00BFFF; // Default Blue

    PinInstance* hitPin = HitTestPin(mousePos);
    if (hitPin && hitPin != draggingStartPin_) {
        if (CheckPinCompatibility(draggingStartPin_, hitPin)) {
            linkColor = 0xFF00FF00; // Green
        }
        else {
            linkColor = 0xFFFF0000; // Red
        }
    }

    CDrawUtil::DrawBezierCurve(startPos, endPos, startIsInput, endIsInput,
        linkColor);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// プロパティエディタの描画
void NodeGraph::DrawPropertyEditors() {
	// ここではImGuiを使ってプロパティエディタを描画する例を示します。
    if (!draggingNode_)
        return;

    VECTOR2 startPinWorldPos = GetPinWorldPos(draggingStartPin_);

    VECTOR2 startPos = { (startPinWorldPos.x - cameraOffset_.x) * zoomScale_,
                        (startPinWorldPos.y - cameraOffset_.y) * zoomScale_ };

    VECTOR2 endPos = { (canvasMousePos_.x - cameraOffset_.x) * zoomScale_,
                      (canvasMousePos_.y - cameraOffset_.y) * zoomScale_ };

    bool startIsInput = draggingStartPin_->IsInput();
    bool endIsInput = !startIsInput;

    CDrawUtil::DrawBezierCurve(startPos, endPos, startIsInput, endIsInput,
        0XFF00BFFF);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ピンのワールド座標を取得
VECTOR2 NodeGraph::GetPinWorldPos(const PinInstance* pin) {
	// まずはピンの親ノードを見つける
    if (!pin)
        return { 0, 0 };

	// 親ノードを見つけて、そこからピンの位置を計算する
    for (const auto& node : graphData_.nodes) {
        if (node.id == pin->parentNodeId) {
            bool isInput = pin->IsInput();
            const auto& pins = isInput ? node.inputPins : node.outputPins;

            int pinIndex = 0;
            for (size_t i = 0; i < pins.size(); i++) {
                if (pins[i].id == pin->id) {
                    pinIndex = (int)i;
                    break;
                }
            }

            float baseYOffset =
                NODE_HEADER_HEIGHT + NODE_PADDING + PIN_SPACING / 2.0f;
            float pinY = node.position.y + baseYOffset + pinIndex * PIN_SPACING;

            float pinX =
                isInput ? node.position.x : (node.position.x + node.GetWidth());

            return { pinX, pinY };
        }
    }

    return { 0, 0 };
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードのヒットテスト
NodeInstance* NodeGraph::HitTestNode(const VECTOR2& canvasPos) {
    for (auto it = graphData_.nodes.rbegin(); it != graphData_.nodes.rend();
        ++it) {
        NodeInstance& node = *it;
        if (node.HitTest(canvasPos)) {
            return &node;
        }
    }
    return nullptr;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ピンのヒットテスト
PinInstance* NodeGraph::HitTestPin(const VECTOR2& canvasPos) {
    for (auto& node : graphData_.nodes) {
        // 入力ピン
        float inputYOffset = NODE_HEADER_HEIGHT + NODE_PADDING + PIN_SPACING / 2.0f;
        for (auto& pin : node.inputPins) {
            VECTOR2 pinPos = { node.position.x, node.position.y + inputYOffset };

            float dx = pinPos.x - canvasPos.x;
            float dy = pinPos.y - canvasPos.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance <= PIN_HIT_RADIUS) {
                return &pin;
            }

            inputYOffset += PIN_SPACING;
        }

		// 出力ピン
        float outputYOffset =
            NODE_HEADER_HEIGHT + NODE_PADDING + PIN_SPACING / 2.0f;
        for (auto& pin : node.outputPins) {
            VECTOR2 pinPos = { node.position.x + node.GetWidth(),
                              node.position.y + outputYOffset };

            float dx = pinPos.x - canvasPos.x;
            float dy = pinPos.y - canvasPos.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance <= PIN_HIT_RADIUS) {
                return &pin;
            }

            outputYOffset += PIN_SPACING;
        }
    }

    return nullptr;
}
//-----------------------------------------------------------------------------