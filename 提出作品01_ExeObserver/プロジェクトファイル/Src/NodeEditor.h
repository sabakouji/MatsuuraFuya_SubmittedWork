#pragma once

#include "Canvas.h"
#include "CoreData.h"
#include "NodeDefinition.h"
#include "NodeGraph.h"
#include "Object3D.h"
#include "Palette.h"
#include "Serializer.h"
#include <algorithm>
#include <atomic>
#include <memory>
#include <vector>

class NodeFactory;

class NodeEditor {
public:
  NodeEditor(std::shared_ptr<NodeFactory> factory,
             std::shared_ptr<Serializer> serializer);
  ~NodeEditor() = default;

  bool Initialize();

  void Update();

  void Draw();

  void HandleInput(const VECTOR2 &mousePos, bool isLeftClickDown,
                   bool isRightClickDown);

  bool LoadGraph(const std::string &filePath);
  bool SaveGraph(const std::string &filePath) const;

  const GraphData &GetGraphData() const { return graph_->GetGraphData(); }

  bool IsValueChanged() const { return graph_->IsValueChanged(); }

private:
  std::shared_ptr<NodeFactory> nodeFactory_;
  std::shared_ptr<Serializer> serializer_;

  std::unique_ptr<Canvas> canvas_;
  std::unique_ptr<Palette> palette_;
  std::unique_ptr<NodeGraph> graph_;

  std::string currentFilePath_;

  std::string draggingTemplateName_;

  float paletteWidth_ = 250.0f;
  VECTOR2 canvasAreaPosition_ = {paletteWidth_, 0.0f};

  std::atomic<ImU64> m_NextlinkId = 3000;

  int palletteheight;
  bool isLCrickDown;
  bool isRCrickDown;
  bool isLCrickUp;
  bool isRCrickUp;
  ImVec2 imPos;
  VECTOR2 mousePos;

  void DrawDraggingNodePreview();
  void InputMouseAction();
  void ImVec2toVECTOR2();
  void RequestDelete();
};
