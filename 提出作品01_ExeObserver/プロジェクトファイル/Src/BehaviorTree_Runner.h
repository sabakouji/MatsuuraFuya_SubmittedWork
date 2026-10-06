#pragma once

#include "BehaviorTree_Nodes.h"
#include "CoreData.h"
#include "NodeDefinition.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

class Executor;

class BehaviorTreeRunner {
public:
  explicit BehaviorTreeRunner() = default;

  bool LoadGraph(const GraphData &graphdata);

  BehaviorState Execute();

  void SetExeContext(Executor *executor) { m_executorContext = executor; };

  void ResetTree();

private:
  BehaviorTreeNode *m_rootNode = nullptr;
  Executor *m_executorContext = nullptr;

  std::vector<BehaviorTreeNode::Nodeptr> ownedNodes;
  std::map<NodeInstance, BehaviorTreeNode *> m_runtimeNodes;
  GraphData m_graphdata;

  const NodeInstance *FindNodeInctanceById(const std::string &nodeId) const;
  const PinInstance *FindPinInstance(const std::string &pinId) const;

  BehaviorTreeNode::Nodeptr
  CreateBehaviorNode(const NodeInstance &nodeInstance);

  bool BulidHierarchy(std::map<std::string, BehaviorTreeNode *> &nodeIdToPtr);
  bool IsActionNode(const NodeInstance &nodeInstance) const;
  bool FindRootNode();
  bool IsCompositeNode(const NodeInstance &nodeInstance) const;
  bool IsConditionNode(const NodeInstance &nodeInstance) const;

public:
  std::string GetInputSourceNodeId(const std::string &nodeId,
                                   const std::string &pinName) const;
  const NodeInstance *GetNodeInstance(const std::string &nodeId) const;

  void RebuildAllPinsMap();
};