#include "CDrawUtil.h"
#include "CoreData.h"
#include "MyImgui.h"
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <string>

//-----------------------------------------------------------------------------
// 定数定義
const float NODE_HEADER_HEIGHT = 30.0f;
const float NODE_PADDING = 10.0f;
const float PIN_SPACING = 30.0f;
const float PIN_RADIUS = 5.0f;
const float PIN_HIT_RADIUS = 7.0f;

const float PREVIEW_WIDTH = 150.0f;
const float PREVIEW_HEADER_HEIGHT = 16.0f;
const float PREVIEW_PIN_SPACING = 16.0f;
const float PREVIEW_PIN_RADIUS = 4.0f;

const int COLOR_NODE_BG = 0xCC1E1E1E;
const int COLOR_NODE_HEADER = 0xFF404040;
const int COLOR_NODE_HEADER_HOVER = 0xFF505050;
const int COLOR_NODE_HEADER_SELECTED = 0xFF606060;
const int COLOR_TEXT = 0xFFFFFFFF;
const int COLOR_PIN_DATA = 0xFF00BFFF;
const int COLOR_PIN_CONTROL = 0xFFFFFFFF;
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスの幅を計算
float NodeInstance::GetWidth() const {
	// 最小幅はタイトルとピンの幅を考慮して120pxに設定
    float minWidth = 120.0f;
    float titleWidth =
        ImGui::CalcTextSize(MyImgui::GetLocalizedText(displayName).c_str()).x +
        30.0f;

	// 入力ピンの幅を計算
    float maxInputWidth = 0.0f;
    for (const auto& pin : inputPins) {
        float w =
            ImGui::CalcTextSize(MyImgui::GetLocalizedText(pin.name).c_str()).x;

		// データピンで、接続されておらず、プロパティにリンクされている場合は、コントロールの幅を考慮
        if (pin.IsDataPin() && !pin.IsConnected() &&
            !pin.linkedPropertyName.empty()) {
            auto defIt = propertyDefinitions.find(pin.linkedPropertyName);
            if (defIt != propertyDefinitions.end()) {
                const PropertyDefinition& propDef = defIt->second;
                float controlWidth = 0.0f;
                switch (propDef.type) {
                case PropertyType::Float:
                case PropertyType::Int:
                    controlWidth = 80.0f; // Fixed width for number inputs
                    break;
                case PropertyType::Bool:
                    controlWidth = 20.0f; // Checkbox width
                    break;
                case PropertyType::Enum: {
                    const EnumDefinition* enumDef =
                        EnumRegistry::GetInstance().GetEnum(propDef.enumTypeName);
                    if (enumDef) {
                        float maxEnumW = 0.0f;
                        for (const auto& val : enumDef->values) {
                            float ew = ImGui::CalcTextSize(val.c_str()).x;
                            if (ew > maxEnumW)
                                maxEnumW = ew;
                        }
                        controlWidth = maxEnumW + 30.0f; // Text + Arrow padding
                    }
                    else {
                        controlWidth = 100.0f; // Fallback
                    }
                    break;
                }
                default:
                    break;
                }
                w += controlWidth + 10.0f;
            }
        }

		// ピン名の幅とコントロールの幅を考慮して最大幅を更新
        if (w > maxInputWidth)
            maxInputWidth = w;
    }

	// 入力ピンがない場合は、プロパティのコントロール幅を考慮
    if (inputPins.empty()) {
        for (const auto& pair : propertyDefinitions) {
            if (!pair.second.isVisible)
                continue;

            const PropertyDefinition& propDef = pair.second;
            float controlWidth = 0.0f;
            switch (propDef.type) {
            case PropertyType::Float:
            case PropertyType::Int:
                controlWidth = 80.0f;
                break;
            case PropertyType::Bool:
                controlWidth = 20.0f;
                break;
            case PropertyType::Enum: {
                const EnumDefinition* enumDef =
                    EnumRegistry::GetInstance().GetEnum(propDef.enumTypeName);
                if (enumDef) {
                    float maxEnumW = 0.0f;
                    for (const auto& val : enumDef->values) {
                        float ew = ImGui::CalcTextSize(val.c_str()).x;
                        if (ew > maxEnumW)
                            maxEnumW = ew;
                    }
                    controlWidth = maxEnumW + 30.0f;
                }
                else {
                    controlWidth = 100.0f;
                }
                break;
            }
            default:
                break;
            }
            if (controlWidth > maxInputWidth)
                maxInputWidth = controlWidth;
        }
    }

	// 出力ピンの幅を計算
    float maxOutputWidth = 0.0f;
    for (const auto& pin : outputPins) {
        float w =
            ImGui::CalcTextSize(MyImgui::GetLocalizedText(pin.name).c_str()).x;
        if (w > maxOutputWidth)
            maxOutputWidth = w;
    }

    float pinsWidth =
        maxInputWidth + maxOutputWidth + (PIN_RADIUS * 4.0f) + 40.0f + 50.0f;
    return std::max<float>(minWidth, std::max<float>(titleWidth, pinsWidth));
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスの高さを計算
float NodeInstance::GetHeight() const {
    float pinCount = std::max<float>(inputPins.size(), outputPins.size());
    float contentHeight = pinCount * PIN_SPACING;
    return NODE_HEADER_HEIGHT + contentHeight + NODE_PADDING * 2;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスを描画
void NodeInstance::Draw(const VECTOR2& cameraOffset, float scale) const {
	// ノードのスクリーン座標を計算
    VECTOR2 screenPos = { (position.x - cameraOffset.x) * scale,
                         (position.y - cameraOffset.y) * scale };

	// ノードの幅と高さをスケールに合わせて計算
    float nodeWidth = GetWidth() * scale;
    float nodeHeight = GetHeight() * scale;

	// スケールに合わせた定数を計算
    float scaledHeaderHeight = NODE_HEADER_HEIGHT * scale;
    float scaledPadding = NODE_PADDING * scale;
    float scaledPinSpacing = PIN_SPACING * scale;
    float scaledPinRadius = PIN_RADIUS * scale;

	// ノードのヘッダーの色を状態に応じて決定
    int headerColor;
    if (isSelected) {
        headerColor = COLOR_NODE_HEADER_SELECTED;
    }
    else if (isHovered) {
        headerColor = COLOR_NODE_HEADER_HOVER;
    }
    else {
        headerColor = COLOR_NODE_HEADER;
    }

	// ノードの背景とヘッダーを描画
    CDrawUtil::DrawRect(screenPos.x, screenPos.y, nodeWidth, nodeHeight,
        COLOR_NODE_BG);
    CDrawUtil::DrawRect(screenPos.x, screenPos.y, nodeWidth, scaledHeaderHeight,
        headerColor);

	// ノードのタイトルを描画
    CDrawUtil::DrawText(MyImgui::GetLocalizedText(displayName).c_str(),
        screenPos.x + scaledPadding,
        screenPos.y + (scaledHeaderHeight - 18.0f) /
        2.0f,
        COLOR_TEXT);

	// 選択されている場合は、ノードの周りに青い枠線を描画
    if (isSelected) {
        ImDrawList* dl = CDrawUtil::GetDrawList();
        ImVec2 pmin(screenPos.x - 2, screenPos.y - 2);
        ImVec2 pmax(screenPos.x + nodeWidth + 2, screenPos.y + nodeHeight + 2);
        dl->AddRect(pmin, pmax, IM_COL32(0, 191, 255, 255), 4.0f, 0, 2.5f);
    }

	// 入力ピンを描画
    float inputYOffset =
        scaledHeaderHeight + scaledPadding + scaledPinSpacing / 2.0f;
    for (const auto& pin : inputPins) {
        bool isControlPin =
            (pin.type == PinType::Control || pin.type == PinType::ControlOut);
        int pinColor = isControlPin ? COLOR_PIN_CONTROL : COLOR_PIN_DATA;

        float pinX = screenPos.x;
        float pinY = screenPos.y + inputYOffset;

        CDrawUtil::DrawCircle(pinX, pinY, scaledPinRadius, pinColor);
        CDrawUtil::DrawTextA(MyImgui::GetLocalizedText(pin.name).c_str(),
            pinX + scaledPinRadius + 5.0f, pinY - 7.0f,
            COLOR_TEXT);

        inputYOffset += scaledPinSpacing;
    }

	// 出力ピンを描画
    float outputYOffset =
        scaledHeaderHeight + scaledPadding + scaledPinSpacing / 2.0f;
    for (const auto& pin : outputPins) {
        bool isControlPin =
            (pin.type == PinType::ControlIn || pin.type == PinType::ControlOut);
        int pinColor = isControlPin ? COLOR_PIN_CONTROL : COLOR_PIN_DATA;

        float pinX = screenPos.x + nodeWidth;
        float pinY = screenPos.y + outputYOffset;

        CDrawUtil::DrawCircle(pinX, pinY, scaledPinRadius, pinColor);

        float textWidth =
            ImGui::CalcTextSize(MyImgui::GetLocalizedText(pin.name).c_str()).x;

        CDrawUtil::DrawText(MyImgui::GetLocalizedText(pin.name).c_str(),
            pinX - scaledPinRadius - 5.0f - textWidth, pinY - 7.0f,
            COLOR_TEXT);

        outputYOffset += scaledPinSpacing;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスのインライン入力を描画
bool NodeInstance::DrawInlineInputs(const VECTOR2& cameraOffset) {
	// インライン入力が必要なピンを描画し、値の変更があった場合はtrueを返す
    bool changed = false;

	// ノードのスクリーン座標を計算
    VECTOR2 screenPos = { position.x - cameraOffset.x,
                         position.y - cameraOffset.y };

	// ノードIDをImGuiのIDスタックにプッシュして、同じIDを持つ他のノードと衝突しないようにする
    ImGui::PushID(id.c_str());

	// 入力ピンの位置を計算して、データピンで接続されておらず、プロパティにリンクされているものに対してインライン入力を描画
    float pinYOffset = NODE_HEADER_HEIGHT + NODE_PADDING + PIN_SPACING / 2.0f;
    float currentNodeWidth = GetWidth();

	// 入力ピンをループして、条件に合うものに対してインライン入力を描画
    for (auto& pin : inputPins) {
		// コントロールピンはスキップ
        if (!pin.IsDataPin()) {
            pinYOffset += PIN_SPACING;
            continue;
        }
		// データピンで、接続されておらず、プロパティにリンクされているものに対してインライン入力を描画
        if (pin.IsConnected()) {
            pinYOffset += PIN_SPACING;
            continue;
        }

		// プロパティ名を取得
        std::string propName = pin.linkedPropertyName;

		// プロパティ名が空の場合はスキップ
        if (propName.empty()) {
            pinYOffset += PIN_SPACING;
            continue;
        }

		// プロパティの値を取得。プロパティが存在しない場合は、デフォルト値を設定してから取得
        auto propIt = properties.find(propName);

		// プロパティが存在しない場合は、プロパティ定義からデフォルト値を取得して設定
        if (propIt == properties.end()) {
            std::string defaultValue = "0";
            auto defIt = propertyDefinitions.find(propName);
            if (defIt != propertyDefinitions.end()) {
                defaultValue = defIt->second.defaultValue;
            }
            properties[propName] = defaultValue;
            propIt = properties.find(propName);
        }

		// プロパティの値を参照
        std::string& propValue = propIt->second;

		// インライン入力の位置を計算
        float pinNameWidth =
            ImGui::CalcTextSize(MyImgui::GetLocalizedText(pin.name).c_str()).x;

		// ピン名がある場合は、ピンの右側にインライン入力を配置し、ない場合はピンのすぐ右に配置
        float inputX;
        if (pin.name.empty()) {
            inputX = screenPos.x + 15.0f;
        }
		// ピン名がある場合は、ピンの右側にインライン入力を配置
        else {
            inputX = screenPos.x + PIN_RADIUS * 8.0f + pinNameWidth + 8.0f;
        }

		// インライン入力のY位置は、ピンのY位置に合わせる
        float inputY = screenPos.y + pinYOffset - 12.0f;

		// ノードの幅からインライン入力のX位置を引いて、利用可能な幅を計算。最小幅は40pxに設定
        float availableWidth = currentNodeWidth - (inputX - screenPos.x) - 10.0f;
        if (availableWidth < 40.0f)
            availableWidth = 40.0f;
        ImGui::SetCursorPos(ImVec2(inputX, inputY));
        ImGui::PushID(pin.id.c_str());

		// プロパティ定義を取得して、プロパティの種類に応じたインライン入力を描画
        auto defIt = propertyDefinitions.find(propName);

		// プロパティ定義が存在する場合は、プロパティの種類に応じたインライン入力を描画
        if (defIt != propertyDefinitions.end()) {
            const PropertyDefinition& propDef = defIt->second;

			// プロパティの種類に応じたインライン入力を描画
            switch (propDef.type) {
            case PropertyType::Float: {
                float value = 0.0f;
                try {
                    value = std::stof(propValue);
                }
                catch (...) {
                }

                ImGui::SetNextItemWidth(availableWidth);

                if (ImGui::DragFloat("##value", &value, 0.1f, propDef.minValue,
                    propDef.maxValue)) {
                    propValue = std::to_string(value);
                    changed = true;
                }
                break;
            }
            case PropertyType::Int: {
                int value = 0;
                try {
                    value = std::stoi(propValue);
                }
                catch (...) {
                }

                ImGui::SetNextItemWidth(availableWidth);

                if (ImGui::DragInt("##value", &value, 1, (int)propDef.minValue,
                    (int)propDef.maxValue)) {
                    propValue = std::to_string(value);
                    changed = true;
                }
                break;
            }
            case PropertyType::Bool: {
                bool value = (propValue == "true" || propValue == "1");
                if (ImGui::Checkbox("##value", &value)) {
                    propValue = value ? "true" : "false";
                    changed = true;
                }
                break;
            }
            case PropertyType::Enum: {
                const EnumDefinition* enumDef =
                    EnumRegistry::GetInstance().GetEnum(propDef.enumTypeName);
                if (enumDef) {
                    ImGui::SetNextItemWidth(availableWidth);
                    if (ImGui::BeginCombo("##value", propValue.c_str())) {
                        for (size_t i = 0; i < enumDef->values.size(); ++i) {
                            bool isSelected = (propValue == enumDef->values[i]);
                            if (ImGui::Selectable(enumDef->values[i].c_str(), isSelected)) {
                                propValue = enumDef->values[i];
                                changed = true;
                            }
                            if (isSelected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                break;
            }
            }
        }
        ImGui::PopID();
        pinYOffset += PIN_SPACING;
    }
    ImGui::PopID();
    return changed;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスのプロパティエディタを描画
std::string NodeInstance::GetPropertyValue(const std::string& propName) const {
    const PinInstance* dataPin = GetPropertyDataPin(propName);
	// データピンが存在し、接続されておらず、プロパティにリンクされている場合は、データピンの値を優先して返す
    if (dataPin && dataPin->IsConnected() && !dataPin->dataValue.empty()) {
        return dataPin->dataValue;
    }

	// それ以外の場合は、プロパティの値を返す。プロパティが存在しない場合は空文字を返す
    auto it = properties.find(propName);
    if (it != properties.end()) {
        return it->second;
    }

    return "";
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスのプロパティにリンクされたデータピンを取得
PinInstance* NodeInstance::GetPropertyDataPin(const std::string& propName) {
	// 入力ピンをループして、プロパティにリンクされたデータピンを探す
    for (auto& pin : inputPins) {
        if (pin.linkedPropertyName == propName) {
            return &pin;
        }
    }
    return nullptr;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスのプロパティにリンクされたデータピンを取得（const版）
const PinInstance*
NodeInstance::GetPropertyDataPin(const std::string& propName) const {
	// 入力ピンをループして、プロパティにリンクされたデータピンを探す
    for (const auto& pin : inputPins) {
        if (pin.linkedPropertyName == propName) {
            return &pin;
        }
    }
    return nullptr;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスのプレビューを描画
void NodeInstance::DrawPreview(const VECTOR2& drawPos, float scale) const {
	// スケールに合わせた定数を計算
    float previewWidth = PREVIEW_WIDTH * scale;
    float previewHeaderHeight = PREVIEW_HEADER_HEIGHT * scale;
    float previewPinRadius = PREVIEW_PIN_RADIUS * scale;
    float previewPinSpacing = PREVIEW_PIN_SPACING * scale;

	// ピンの数に応じてプレビューの高さを計算
    float pinCount = std::max<float>(inputPins.size(), outputPins.size());
    float contentHeight = pinCount * previewPinSpacing;
    float previewHeight = previewHeaderHeight + contentHeight + 10.0f * scale;

	// ヘッダーの色を状態に応じて決定
    int headerColor = COLOR_NODE_HEADER;
    if (isHovered) {
        headerColor = COLOR_NODE_HEADER_HOVER;
    }

	// ノードの背景とヘッダーを描画
    CDrawUtil::DrawRect(drawPos.x, drawPos.y, previewWidth, previewHeight,
        COLOR_NODE_BG);
    CDrawUtil::DrawRect(drawPos.x, drawPos.y, previewWidth, previewHeaderHeight,
        headerColor);
    CDrawUtil::DrawText(
        MyImgui::GetLocalizedText(displayName).c_str(), drawPos.x + 8.0f * scale,
        drawPos.y + (previewHeaderHeight - 16.0f * scale) / 2.0f, COLOR_TEXT);

	// ホバーされている場合は、ノードの周りに青い枠線を描画
    if (isHovered) {
        ImDrawList* dl = CDrawUtil::GetDrawList();
        ImVec2 pmin(drawPos.x - 1, drawPos.y - 1);
        ImVec2 pmax(drawPos.x + previewWidth + 1, drawPos.y + previewHeight + 1);
        dl->AddRect(pmin, pmax, IM_COL32(0, 191, 255, 255), 4.0f, 0, 2.0f);
    }

	// 入力ピンを描画
    float inputYOffset = previewHeaderHeight + 8.0f * scale;
    for (const auto& pin : inputPins) {
        bool isControlPin =
            (pin.type == PinType::ControlIn || pin.type == PinType::ControlOut);
        int pinColor = isControlPin ? COLOR_PIN_CONTROL : COLOR_PIN_DATA;

        CDrawUtil::DrawCircle(drawPos.x, drawPos.y + inputYOffset, previewPinRadius,
            pinColor);

        inputYOffset += previewPinSpacing;
    }

	// 出力ピンを描画
    float outputYOffset = previewHeaderHeight + 8.0f * scale;
    for (const auto& pin : outputPins) {
        bool isControlPin =
            (pin.type == PinType::ControlIn || pin.type == PinType::ControlOut);
        int pinColor = isControlPin ? COLOR_PIN_CONTROL : COLOR_PIN_DATA;

        CDrawUtil::DrawCircle(drawPos.x + previewWidth, drawPos.y + outputYOffset,
            previewPinRadius, pinColor);

        outputYOffset += previewPinSpacing;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードのヒットテスト（マウスポインタがノードの矩形内にあるかを判定）
bool NodeInstance::HitTest(const VECTOR2& point) const {
    float nodeWidth = GetWidth();
    float nodeHeigth = GetHeight();
    return (point.x >= position.x && point.x <= position.x + nodeWidth &&
        point.y >= position.y && point.y <= position.y + nodeHeigth);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ピンのヒットテスト（マウスポインタがどのピンに近いかを判定）
PinInstance* NodeInstance::HitTestPin(const VECTOR2& point) {
	// ノードの幅を取得
    float nodeWidth = GetWidth();

	// 入力ピンをループして、マウスポインタがどのピンに近いかを判定
    float inputYOffset = NODE_HEADER_HEIGHT + NODE_PADDING + PIN_SPACING / 2.0f;
    for (auto& pin : inputPins) {
        VECTOR2 pinPos = { position.x, position.y + inputYOffset };

        float dx = point.x - pinPos.x;
        float dy = point.y - pinPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= PIN_HIT_RADIUS) { // Changed to Constant specific
            return &pin;
        }
        inputYOffset += PIN_SPACING;
    }

	// 出力ピンをループして、マウスポインタがどのピンに近いかを判定
    float outputYOffset = NODE_HEADER_HEIGHT + NODE_PADDING + PIN_SPACING / 2.0f;
    for (auto& pin : outputPins) {
        VECTOR2 pinPos = { position.x + nodeWidth, position.y + outputYOffset };

        float dx = point.x - pinPos.x;
        float dy = point.y - pinPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= PIN_HIT_RADIUS) {
            return &pin;
        }

        outputYOffset += PIN_SPACING;
    }

    return nullptr;
}
//-----------------------------------------------------------------------------