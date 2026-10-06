#include "NodeEditor.h"
#include "CDrawUtil.h"
#include "MyImgui.h"
#include <iostream>

//-----------------------------------------------------------------------------
// 定数
constexpr float DRAG_PREVIEW_WIDTH = 150.0f;
constexpr float DRAG_PREVIEW_HEADER_HEIGHT = 30.0f;
constexpr float DRAG_PREVIEW_PIN_SPACING = 20.0f;
constexpr float DRAG_PREVIEW_PADDING = 10.0f;
constexpr float DRAG_PREVIEW_PIN_RADIUS = 5.0f;
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// NodeEditorクラスの実装
NodeEditor::NodeEditor(std::shared_ptr<NodeFactory> factory,
                       std::shared_ptr<Serializer> serializer)
    : nodeFactory_(std::move(factory)), serializer_(std::move(serializer)) {
  currentFilePath_ = "default_graph.json";

  isLCrickDown = false;
  isRCrickDown = false;

  canvas_ = std::make_unique<Canvas>();
  graph_ = std::make_unique<NodeGraph>(nodeFactory_);
  graph_->SetOnGraphChanged([this]() {
    if (!currentFilePath_.empty()) {
      SaveGraph(currentFilePath_);
      std::cout << "Auto-saved to " << currentFilePath_ << std::endl;
    }
  });

  palette_ =
      std::make_unique<Palette>(nodeFactory_, paletteWidth_, graph_.get());
}

bool NodeEditor::Initialize() {
  std::cout << "INFO: NodeEditor Initialized." << std::endl;

  nodeFactory_->LoadDefinitionsFromJson("behavior_nodes.json");

  std::string defaultPath = "default_graph.json";

  bool loadSuccess = LoadGraph(defaultPath);

  currentFilePath_ = defaultPath;

  return true;
}

void NodeEditor::Update() {
  imPos = ImGui::GetMousePos();

  ImVec2toVECTOR2();
  InputMouseAction();

  RequestDelete();

  HandleInput(mousePos, isLCrickDown, isRCrickDown);
}

void NodeEditor::Draw() {
  // Pass camera state to canvas for scrolling background
  if (graph_ && canvas_) {
    canvas_->Draw(graph_->GetCameraOffset(), graph_->GetZoomScale());
  }

  if (graph_) {
    graph_->Draw();
  }

  if (palette_) {
    palette_->Draw();
  }
}

void NodeEditor::ImVec2toVECTOR2() {
  mousePos.x = static_cast<float>(imPos.x);
  mousePos.y = static_cast<float>(imPos.y);
}

void NodeEditor::RequestDelete() {}

void NodeEditor::InputMouseAction() {
  bool diLeft = GameDevice()->m_pDI->CheckMouse(KD_DAT, DIM_LBUTTON);
  bool diRight = GameDevice()->m_pDI->CheckMouse(KD_DAT, DIM_RBUTTON);

  isLCrickDown = diLeft;
  isRCrickDown = diRight;

  isLCrickUp = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
  isRCrickUp = ImGui::IsMouseReleased(ImGuiMouseButton_Right);

  if (diLeft) {
    std::cout << "ButtonDown" << "\n";
  }

  bool imguiDelete = GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_DELETE);
  bool imguictrl = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);

  if (imguiDelete && graph_) {
    if (imguictrl) {
      graph_->DeleteAllNodes();
    } else {
      graph_->DeleteSelectedNode();
    }
  }
}

void NodeEditor::HandleInput(const VECTOR2 &mousePos, bool isLeftClickDown,
                             bool isRightClickDown) {
  palette_->HandleInput(mousePos, isLeftClickDown);

  graph_->HandleInput(mousePos, isLeftClickDown, isRightClickDown);
}

void NodeEditor::DrawDraggingNodePreview() {
  ImVec2 mousePos = ImGui::GetMousePos();

  // テンプレート情報を取得
  const auto &templates = nodeFactory_->GetAllTemplates();
  auto it = templates.find(draggingTemplateName_);

  if (it == templates.end())
    return;

  const NodeTemplate &tmpl = it->second;

  // ノードの高さを計算
  float pinCount = std::max<float>(tmpl.inputs.size(), tmpl.outputs.size());
  float contentHeight = pinCount * DRAG_PREVIEW_PIN_SPACING;
  float nodeHeight =
      DRAG_PREVIEW_HEADER_HEIGHT + contentHeight + DRAG_PREVIEW_PADDING * 2;

  // プレビュー位置（マウスカーソルの中心に表示）
  float previewX = mousePos.x - DRAG_PREVIEW_WIDTH / 2.0f;
  float previewY = mousePos.y - DRAG_PREVIEW_HEADER_HEIGHT / 2.0f;

  // 半透明色
  constexpr int ALPHA = 0xAA; // 約67%透明度
  int bgColor = (ALPHA << 24) | 0x323232;
  int headerColor = (ALPHA << 24) | 0x00BFFF;
  int textColor = 0xFFFFFFFF;
  int pinControlColor = (ALPHA << 24) | 0x00FF00;
  int pinDataColor = (ALPHA << 24) | 0xFFFF00;

  // ノード本体の背景
  CDrawUtil::DrawRect(previewX, previewY, DRAG_PREVIEW_WIDTH, nodeHeight,
                      bgColor);

  // ノードヘッダー
  CDrawUtil::DrawRect(previewX, previewY, DRAG_PREVIEW_WIDTH,
                      DRAG_PREVIEW_HEADER_HEIGHT, headerColor);

  // ノード名
  CDrawUtil::DrawText(MyImgui::GetLocalizedText(tmpl.displayName).c_str(),
                      previewX + DRAG_PREVIEW_PADDING,
                      previewY + (DRAG_PREVIEW_HEADER_HEIGHT - 18.0f) / 2.0f,
                      textColor);

  // 入力ピンの描画
  float inputYOffset = DRAG_PREVIEW_HEADER_HEIGHT + DRAG_PREVIEW_PADDING +
                       DRAG_PREVIEW_PIN_SPACING / 2.0f;
  for (const auto &pin : tmpl.inputs) {
    bool isControlPin =
        (pin.type == PinType::ControlIn || pin.type == PinType::ControlOut);
    int pinColor = isControlPin ? pinControlColor : pinDataColor;

    float pinX = previewX;
    float pinY = previewY + inputYOffset;

    CDrawUtil::DrawCircle(pinX, pinY, DRAG_PREVIEW_PIN_RADIUS, pinColor);
    CDrawUtil::DrawText(MyImgui::GetLocalizedText(pin.name).c_str(),
                        pinX + DRAG_PREVIEW_PIN_RADIUS + 5.0f, pinY - 7.0f,
                        textColor);

    inputYOffset += DRAG_PREVIEW_PIN_SPACING;
  }

  // 出力ピンの描画
  float outputYOffset = DRAG_PREVIEW_HEADER_HEIGHT + DRAG_PREVIEW_PADDING +
                        DRAG_PREVIEW_PIN_SPACING / 2.0f;
  for (const auto &pin : tmpl.outputs) {
    bool isControlPin =
        (pin.type == PinType::ControlIn || pin.type == PinType::ControlOut);
    int pinColor = isControlPin ? pinControlColor : pinDataColor;

    float pinX = previewX + DRAG_PREVIEW_WIDTH;
    float pinY = previewY + outputYOffset;

    CDrawUtil::DrawCircle(pinX, pinY, DRAG_PREVIEW_PIN_RADIUS, pinColor);
    CDrawUtil::DrawText(MyImgui::GetLocalizedText(pin.name).c_str(),
                        pinX - DRAG_PREVIEW_PIN_RADIUS - 55.0f, pinY - 7.0f,
                        textColor);

    outputYOffset += DRAG_PREVIEW_PIN_SPACING;
  }

  // グリッドスナップのガイド線（オプション）
  ImDrawList *dl = ImGui::GetForegroundDrawList();
  dl->AddRect(
      ImVec2(previewX - 1, previewY - 1),
      ImVec2(previewX + DRAG_PREVIEW_WIDTH + 1, previewY + nodeHeight + 1),
      IM_COL32(0, 191, 255, 180), 4.0f, 0, 2.0f);
}

bool NodeEditor::LoadGraph(const std::string &filePath) {
  GraphData loadedGraphData;

  // 処理をSerializerに委譲
  if (!serializer_->Load(filePath, loadedGraphData)) {
    return false;
  }
  currentFilePath_ = filePath;
  graph_->SetGraphData(std::move(loadedGraphData));

  return true;
}

bool NodeEditor::SaveGraph(const std::string &filePath) const {
  if (const_cast<NodeEditor *>(this)->currentFilePath_ != filePath) {
    const_cast<NodeEditor *>(this)->currentFilePath_ = filePath;
  }
  const GraphData &graphData = graph_->GetGraphData();

  return serializer_->Save(graphData, filePath);
}