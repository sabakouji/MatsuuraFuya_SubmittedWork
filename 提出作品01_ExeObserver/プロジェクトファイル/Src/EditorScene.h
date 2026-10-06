#pragma once
#include "SceneBase.h"
#include "NodeDefinition.h"
#include "BehaviorTree_Runner.h"
#include "Serializer.h"
#include <memory>

class NodeEditor;

class EditorScene : public SceneBase
{
public:
	EditorScene();
	~EditorScene();
	void Update() override;
	void Draw() override;

	void ShutDown();
private:
	std::shared_ptr<NodeFactory> m_factory;
	std::shared_ptr<Serializer> m_serializer;
	std::unique_ptr<NodeEditor> m_editor;
	std::unique_ptr<BehaviorTreeRunner> m_runner;

	VECTOR2 mousePos_ = { 0.0f, 0.0f };
};
