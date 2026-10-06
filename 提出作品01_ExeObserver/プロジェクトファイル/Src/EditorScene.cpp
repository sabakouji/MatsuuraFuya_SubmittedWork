#include "EditorScene.h"
#include "FadeObject.h"
#include "NodeDefinition.h"
#include "NodeEditor.h"
#include "ObjectManager.h"
#include "AudioManager.h"
#include <iostream>

//-----------------------------------------------------------------------------
// 初期化
EditorScene::EditorScene()
    : m_factory(std::make_shared<NodeFactory>()),
    m_serializer(std::make_shared<Serializer>()) {
    std::cout << "EditorScene: Initialize" << std::endl;

    m_editor = std::make_unique<NodeEditor>(m_factory, m_serializer);
    m_runner = std::make_unique<BehaviorTreeRunner>();

    // 自動ロード処理 (デバッグ用)
    m_editor->LoadGraph("default_graph.json");

    // フェードイン
    FadeObject* fade = ObjectManager::FindGameObject<FadeObject>();
    if (fade) {
        fade->StartFadeIn(1.0f, [fade]() {
            ObjectManager::DontDestroy(fade, false);
            ObjectManager::Destroy(fade);
            });
    }

    // BGM再生
    AudioManager::StopAll();
    AudioManager::Audio("BGM_Editor")->Play(AUDIO_LOOP);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 終了処理
EditorScene::~EditorScene() { ShutDown(); }
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 更新処理
void EditorScene::Update() {
    //もしエディタが存在するなら更新する
    if (m_editor) {
        m_editor->Update();
    }

    // ライブアップデート: エディタの内容が変更されたら、ビヘイビアツリーを再ロードする
    if (m_editor && m_editor->IsValueChanged()) {
        if (m_runner) {
            m_runner->LoadGraph(m_editor->GetGraphData());

            std::cout << "Live Update: Graph reloaded." << std::endl;
        }
    }

    // シーン遷移の入力処理
    if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_N)) {
        if (m_editor) {
            m_editor->SaveGraph("default_graph.json");
        }
        SceneManager::ChangeScene("PlayScene");
    }

    // エディタからのシーン遷移
    if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_ESCAPE)) {
        if (m_editor) {
            m_editor->SaveGraph("default_graph.json");
        }

        SceneManager::ReturnScene();
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 描画処理
void EditorScene::Draw() {
    // エディタの描画
    if (m_editor) {
        m_editor->Draw();
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// エディターを閉じる処理
void EditorScene::ShutDown() {
	// 自動保存処理 (デバッグ用)
    if (!m_editor)
        return;

	// シーンを離れる前に、エディタの内容を保存する
    if (!m_editor->SaveGraph("default_graph.json")) {
        std::cerr << "ERROR: Failed to save default_graph.json\n";
    }
}
//-----------------------------------------------------------------------------