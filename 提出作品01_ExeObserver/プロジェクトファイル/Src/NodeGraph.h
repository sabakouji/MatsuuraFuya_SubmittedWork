#pragma once
#include "CoreData.h"
#include "GameObject.h"
#include "InlineInput.h"
#include "Object3D.h"
#include <functional>
#include <map>
#include <memory>


class NodeFactory;

class NodeGraph {
public:
  NodeGraph(std::shared_ptr<NodeFactory> factory);
  ~NodeGraph() = default;

  void Draw();

  void HandleInput(const VECTOR2 &mousePos, bool isLeftClickDown,
                   bool isRightClickDown);

  void AddNode(const std::string &templateName, const VECTOR2 &screenPosition);

  void CreateLink(const std::string &startPinId, const std::string &endPinId);

  const GraphData &GetGraphData() const { return graphData_; }
  void SetGraphData(GraphData &&data);

  PinInstance *GetPinById(const std::string &pinId);

  void StartNodeDrag(const std::string &templateName,
                     const VECTOR2 &screenPosition);

  void DeleteSelectedNode();
  void DeleteAllNodes();

  NodeInstance *GetDraggingNode() { return draggingNode_; };
  NodeInstance *GetSelectedNode() { return selectedNode_; };

  bool IsValueChanged() const { return lineManager_.IsValueChanged(); }

  // Check if a Root node already exists in the graph
  bool HasRootNode() const;

  VECTOR2 GetCameraOffset() const { return cameraOffset_; }
  float GetZoomScale() const { return zoomScale_; }

  // --------------------------------------------------------------------------
  // 座標変換ヘルパー
  // --------------------------------------------------------------------------
  // スクリーン座標(マウス位置など) -> キャンバス座標(ノード位置)
  VECTOR2 ScreenToCanvas(const VECTOR2 &screenPos) const;

  // キャンバス座標(ノード位置) -> スクリーン座標(描画用)
  VECTOR2 CanvasToScreen(const VECTOR2 &canvasPos) const;

  std::function<void()> onGraphChanged_;

  void SetOnGraphChanged(std::function<void()> callback) {
    onGraphChanged_ = callback;
  }

private:
  GraphData graphData_;
  std::shared_ptr<NodeFactory> nodeFactory_;
  InlineInputManager lineManager_;

  bool isDraggingNewNode_;
  bool wasLeftClickDown_;
  bool wasRightClickDown_;

  // --------------------------------------------------------------------------
  // 座標・操作系メンバ変数
  // --------------------------------------------------------------------------
  VECTOR2 cameraOffset_ = {0.0f, 0.0f}; // キャンバスの原点オフセット
  float zoomScale_ = 1.0f;              // 拡大縮小率 (1.0 = 100%)

  // パン(移動)操作用
  bool isPanning_ = false;
  VECTOR2 panStartMousePos_ = {0.0f, 0.0f};
  VECTOR2 panStartCameraOffset_ = {0.0f, 0.0f};

  NodeInstance *selectedNode_;
  NodeInstance *draggingNode_;
  PinInstance *draggingStartPin_;
  VECTOR2 lastMousePos_;
  VECTOR2 canvasMousePos_;

  bool showPropertyPanel_;
  VECTOR2 propertyPanelPos_;
  NodeInstance *propertyPanelNode_;

  // 描画ヘルパー関数 (抽象化: 実際のDirectX描画コマンド)
  void DrawLink(const LinkInstance &link);
  void DrawDraggingLink(const VECTOR2 &mousePos);
  void DrawPropertyEditors();

  VECTOR2 GetPinWorldPos(const PinInstance *pin);

  NodeInstance *HitTestNode(const VECTOR2 &screenPos);
  PinInstance *HitTestPin(const VECTOR2 &screenPos);

  // 内部データ整合性の更新 (リンク作成/削除時に呼ばれる)
  void UpdateAllPinsMap();
  void ClearNodeSelection();
  void DeleteNodeById(const std::string &id);

  void PropagateDataLinkValues();
  void RestorePropertyDefinitionsFromTemplates();

  void RegisterInlineInputFields();
};
