#include "Palette.h"
#include "CDrawUtil.h"
#include "MyImgui.h"
#include <algorithm>
#include <iostream>
#include <sstream>

//-----------------------------------------------------------------------------
// 定数定義
constexpr float NODE_PREVIEW_SPACING_X = 10.0f;
constexpr float NODE_PREVIEW_SPACING_Y = 10.0f;
constexpr float TAB_HEIGHT = 30.0f;
constexpr float TAB_WIDTH = 120.0f;
constexpr float PALETTE_PADDING = 10.0f;
constexpr int NODES_PER_COLUMN = 2;
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 色定義
namespace PaletteColors {
    const int COLOR_TAB_INACTIVE = 0xFF404040;
    const int COLOR_TAB_ACTIVE = 0xFF00BFFF;
    const int COLOR_TAB_HOVER = 0xFF505050;
    const int COLOR_CATEGORY_TEXT = 0xFFFFFFFF;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 初期化
Palette::Palette(std::shared_ptr<NodeFactory> factory, float width,
    NodeGraph* graph)
    : nodeFactory_(std::move(factory)), width_(width),
    selectedCategoryIndex_(0), graph_(graph) {
    CategorizeTemplates();
    CreatePreviewNodes();

    paletteimage = new CSpriteImage(GameDevice()->m_pShader);
    paletteimage->Load("Data/Image/PaletteImage_2.png");
    palettePos = VECTOR2(WINDOW_WIDTH - paletteimage->m_dwImageWidth,
        WINDOW_HEIGHT - paletteimage->m_dwImageHeight);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードテンプレートをカテゴリごとに分類
void Palette::CategorizeTemplates() {
    const auto& templates = nodeFactory_->GetAllTemplates();
	// すべてのテンプレートをループしてカテゴリごとに分類
    for (const auto& pair : templates) {
        const NodeTemplate& tmpl = pair.second;

        // 新しいカテゴリの場合、リストに追加
        if (categorizedTemplates_.find(tmpl.category) ==
            categorizedTemplates_.end()) {
            categoryOrder_.push_back(tmpl.category);
        }

        categorizedTemplates_[tmpl.category].push_back(&tmpl);
    }
    std::cout << "INFO: Palette templates categorized. Total categories: "
        << categorizedTemplates_.size() << std::endl;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 各テンプレートからプレビューノードを作成
void Palette::CreatePreviewNodes() {
	// すべてのテンプレートをループしてプレビューノードを作成
    long long dummyId = 0;

	// カテゴリごとにノードを作成
    for (const auto& categoryPair : categorizedTemplates_) {
        const std::string& category = categoryPair.first;

        for (const NodeTemplate* tmpl : categoryPair.second) {
            auto nodePtr =
                nodeFactory_->CreateNode(tmpl->typeID, VECTOR2(0, 0), dummyId);

            if (nodePtr) {
                previewNodes_[category].push_back(std::move(*nodePtr));
            }
        }
    }
    std::cout << "INFO: Created " << dummyId << " preview node instances."
        << std::endl;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 描画
void Palette::Draw() {
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    CDrawUtil::SetDrawList(dl);

    if (paletteimage && paletteimage->m_pTexture) {
        dl->AddImage((ImTextureID)paletteimage->m_pTexture.Get(),
                     ImVec2(palettePos.x, palettePos.y), 
                     ImVec2(palettePos.x + paletteimage->m_dwImageWidth, palettePos.y + paletteimage->m_dwImageHeight));
    } else {
        CSprite spr;
        spr.Draw(paletteimage, palettePos.x, palettePos.y, 0, 0,
            paletteimage->m_dwImageWidth, paletteimage->m_dwImageHeight);
    }

    CDrawUtil::DrawRect(palettePos.x, palettePos.y, width_, 1000.0f, 0xFF1A1A1A);

    DrawTabs(palettePos);

    if (selectedCategoryIndex_ >= 0 &&
        selectedCategoryIndex_ < categoryOrder_.size()) {
        const std::string& selectedCategory =
            categoryOrder_[selectedCategoryIndex_];
        const auto it = previewNodes_.find(selectedCategory);

        if (it != previewNodes_.end()) {
            VECTOR2 contentPos = { palettePos.x + PALETTE_PADDING,
                                  palettePos.y + TAB_HEIGHT + PALETTE_PADDING };

            DrawNodeGrid(it->second, contentPos);
        }
    }

    CDrawUtil::SetDrawList(nullptr);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// マウス入力処理 (ドラッグ開始の検出)
std::string Palette::HandleInput(const VECTOR2& mousePos,
    bool isLeftClickDown) 
{
	// ドラッグ開始の検出
    if (!isLeftClickDown) {
        return "";
    }
	// ドラッグ中のノードがある場合は新たなドラッグを開始しない
    if (graph_->GetDraggingNode()) {
        return "";
    }

	// マウス位置を取得
    ImVec2 mp = ImGui::GetMousePos();
    VECTOR2 mouse = { mp.x, mp.y };

    // タブのクリック判定
    float tabX = palettePos.x + PALETTE_PADDING;
    float tabY = palettePos.y + PALETTE_PADDING;

	// タブのクリック判定
    for (size_t i = 0; i < categoryOrder_.size(); i++) {
        if (mousePos.x >= tabX && mousePos.x <= tabX + TAB_WIDTH &&
            mousePos.y >= tabY && mousePos.y <= tabY + TAB_HEIGHT) {
            selectedCategoryIndex_ = i;
            std::cout << "INFO: *** Category tab SELECTED: " << categoryOrder_[i]
                << " ***" << std::endl;
            return "";
        }
        tabX += TAB_WIDTH + 5.0f;
    }

	// ノードプレビューのクリック判定
    if (selectedCategoryIndex_ >= 0 &&
        selectedCategoryIndex_ < categoryOrder_.size()) {
        const std::string& selectedCategory =
            categoryOrder_[selectedCategoryIndex_];
        auto it = previewNodes_.find(selectedCategory);

		// ノードプレビューのクリック判定
        if (it != previewNodes_.end() && graph_) {
            for (const auto& node : it->second) {
                if (node.isHovered) {
                    // Prevent creating duplicate Root node
                    if (node.typeId == "Root" && graph_->HasRootNode()) {
                        return "";
                    }

                    graph_->StartNodeDrag(node.typeId, mousePos);

                    return node.typeId;
                }
            }
        }
    }

    return "";
}
//-----------------------------------------------------------------------------

void Palette::DrawNodeGrid(std::vector<NodeInstance>& nodes,
    const VECTOR2& startPos) {
	//マウス位置を取得
    ImVec2 mousePos = ImGui::GetMousePos();
    VECTOR2 mouse = { mousePos.x, mousePos.y };

	// ノードをグリッド状に配置して描画
    int column = 0;
    int row = 0;

	// ノードプレビューのサイズ (固定値)
    constexpr float PREVIEW_WIDTH = 120.0f;
    constexpr float PREVIEW_HEIGHT = 80.0f;

	// ノードをグリッド状に配置して描画
    for (auto& node : nodes) {
		// ノードの位置を計算
        float nodeX =
            startPos.x + column * (PREVIEW_WIDTH + NODE_PREVIEW_SPACING_X);
        float nodeY = startPos.y + row * (PREVIEW_HEIGHT + NODE_PREVIEW_SPACING_Y);

		// ノードが無効な場合のオーバーレイ表示 (例: Rootノードが既に存在する場合)
        bool isDisabled =
            (node.typeId == "Root" && graph_ && graph_->HasRootNode());

		// ノードのホバー状態を更新
        if (isDisabled) {
            node.isHovered = false;
        }
		// ノードのホバー状態を更新
        else {
            node.isHovered = (mouse.x >= nodeX && mouse.x <= nodeX + PREVIEW_WIDTH &&
                mouse.y >= nodeY && mouse.y <= nodeY + PREVIEW_HEIGHT);
        }

		// ノードのプレビューを描画
        node.DrawPreview(VECTOR2(nodeX, nodeY), 1.0f);

		// ノードが無効な場合は半透明の黒いオーバーレイを描画
        if (isDisabled) {
            CDrawUtil::DrawRect(nodeX, nodeY, PREVIEW_WIDTH, PREVIEW_HEIGHT,
                0xAA000000);
        }

		// 次の行に移動
        row++;
        if (row >= NODES_PER_COLUMN) {
            row = 0;
            column++;
        }
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// タブの描画
void Palette::DrawTabs(const VECTOR2& position) {
	// マウス位置を取得
    ImVec2 mousePos = ImGui::GetMousePos();
    VECTOR2 mouse = { mousePos.x, mousePos.y };

	// タブの位置を計算
    float tabX = position.x + PALETTE_PADDING;
    float tabY = position.y + PALETTE_PADDING;

	// タブを描画
    for (size_t i = 0; i < categoryOrder_.size(); i++) {
		// タブのカテゴリ名を取得
        const std::string& category = categoryOrder_[i];

		// タブの状態を判定 (アクティブ、ホバー)
        bool isActive = (i == selectedCategoryIndex_);
        bool isHovered = (mouse.x >= tabX && mouse.x <= tabX + TAB_WIDTH &&
            mouse.y >= tabY && mouse.y <= tabY + TAB_HEIGHT);

		// タブの色を状態に応じて設定
        int tabColor = PaletteColors::COLOR_TAB_INACTIVE;
		// タブの色を状態に応じて設定
        if (isActive) {
            tabColor = PaletteColors::COLOR_TAB_ACTIVE;
        }
		// タブの色を状態に応じて設定
        else if (isHovered) {
            tabColor = PaletteColors::COLOR_TAB_HOVER;
        }

		// タブの背景を描画
        CDrawUtil::DrawRect(tabX, tabY, TAB_WIDTH, TAB_HEIGHT, tabColor);
        CDrawUtil::DrawText(category.c_str(), tabX + 8.0f,
            tabY + (TAB_HEIGHT - 16.0f) / 2.0f,
            PaletteColors::COLOR_CATEGORY_TEXT);

		// アクティブなタブには下線を描画
        if (isActive) {
            ImDrawList* dl = CDrawUtil::GetDrawList();
            dl->AddLine(ImVec2(tabX, tabY + TAB_HEIGHT),
                ImVec2(tabX + TAB_WIDTH, tabY + TAB_HEIGHT),
                IM_COL32(0, 191, 255, 255), 3.0f);
        }

		// 次のタブの位置に移動
        tabX += TAB_WIDTH + 5.0f;
    }
}
//-----------------------------------------------------------------------------