#pragma once
#include "CoreData.h"

class Executor;

class BehaviorTreeNode {
public:
  using Nodeptr = std::unique_ptr<BehaviorTreeNode>;

  explicit BehaviorTreeNode(const NodeInstance &id, const std::string &name)
      : m_id(id), m_name(name) {}

  virtual ~BehaviorTreeNode() = default;

  virtual void Initialize() {};

  virtual BehaviorState Execute(Executor *executorContext) = 0;

  void Reset() { m_state = BehaviorState::READY; };
  void AddChild(Nodeptr child) { m_children.push_back(std::move(child)); }

  // 次のノード（ControlOutで接続されたアクションノード用）
  void SetNextNode(Nodeptr next) { m_nextNode = std::move(next); }
  BehaviorTreeNode *GetNextNode() const { return m_nextNode.get(); }

  const NodeInstance &GetId() const { return m_id; }

protected:
  NodeInstance m_id;
  std::string m_name;
  BehaviorState m_state = BehaviorState::READY;
  std::vector<Nodeptr> m_children;
  Nodeptr m_nextNode;
};

class SelectorNode : public BehaviorTreeNode {
public:
  explicit SelectorNode(const NodeInstance &id, const std::string &name);
  void Initialize() override;
  BehaviorState Execute(Executor *executorContext) override;

private:
  size_t m_currentChildIndex = 0;
};

class SequenceNode : public BehaviorTreeNode {
public:
  explicit SequenceNode(const NodeInstance &id, const std::string &name);
  void Initialize() override;
  BehaviorState Execute(Executor *executorContext) override;

private:
  size_t m_currentChildIndex = 0;
};

class RootNode : public BehaviorTreeNode {
public:
  explicit RootNode(const NodeInstance &id, const std::string &name);
  BehaviorState Execute(Executor *executorContext) override;
};

class ActionNode : public BehaviorTreeNode {
public:
  explicit ActionNode(const NodeInstance &id, const std::string &name)
      : BehaviorTreeNode(id, name) {};

  // アクションの実行結果を取得
  virtual BehaviorState ExecuteAction(Executor *executorContext) {
    return BehaviorState::SUCCESS;
  };

  // 実行（SUCCESSなら次のノードも実行）
  BehaviorState Execute(Executor *executorContext) override;
};

// 条件分岐用ノード基底クラス
class ConditionNode : public BehaviorTreeNode {
public:
  explicit ConditionNode(const NodeInstance &id, const std::string &name)
      : BehaviorTreeNode(id, name), m_trueNode(nullptr), m_falseNode(nullptr) {}

  void SetTrueNode(Nodeptr node) { m_trueNode = std::move(node); }
  void SetFalseNode(Nodeptr node) { m_falseNode = std::move(node); }

  BehaviorTreeNode *GetTrueNode() const { return m_trueNode.get(); }
  BehaviorTreeNode *GetFalseNode() const { return m_falseNode.get(); }

protected:
  Nodeptr m_trueNode;
  Nodeptr m_falseNode;
};

class IfNode : public ConditionNode {
public:
  explicit IfNode(const NodeInstance &id, const std::string &name);
  BehaviorState Execute(Executor *executorContext) override;
};
