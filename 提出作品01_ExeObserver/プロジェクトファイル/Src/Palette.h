#pragma once

#include "CoreData.h"
#include "NodeDefinition.h"
#include "NodeGraph.h"
#include <memory>
#include <string>
#include <map>

class Palette
{
public:
    Palette(std::shared_ptr<NodeFactory> factory, float width, NodeGraph* graph);
    ~Palette() = default;

    // 描画
    void Draw();

    // マウス入力処理 (ドラッグ開始の検出)
    std::string HandleInput(const VECTOR2& mousePos, bool isLeftClickDown);
private:
    std::shared_ptr<NodeFactory> nodeFactory_;
    std::map<std::string, std::vector<const NodeTemplate*>> categorizedTemplates_;
    std::vector<std::string> categoryOrder_;
    size_t selectedCategoryIndex_;
    std::map<std::string, std::vector<NodeInstance>> previewNodes_;

    float width_;
    VECTOR2 palettePos;

    // 描画ヘルパー関数
    void DrawTabs(const VECTOR2& position);
    void DrawNodeGrid(std::vector<NodeInstance>& nodes, const VECTOR2& startPos);
    void CategorizeTemplates();
    void CreatePreviewNodes();

    CSpriteImage* paletteimage;
    NodeGraph* graph_;
};