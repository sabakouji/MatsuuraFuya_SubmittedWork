#include "BehaviorTree_Nodes.h"
#include "Executor.h"
#include <iostream>

//-----------------------------------------------------------------------------
// シーケンスノードの初期化 
SequenceNode::SequenceNode(const NodeInstance &id, const std::string &name)
    : BehaviorTreeNode(id, name) {}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// シーケンスノードの初期化実行
void SequenceNode::Initialize() {
  BehaviorTreeNode::Initialize();
  m_currentChildIndex = 0;
  m_state = BehaviorState::READY;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// シーケンスノードの実行
BehaviorState SequenceNode::Execute(Executor *executorContext) {
	// READY状態なら実行開始
    if (m_state == BehaviorState::READY) {
		// デバッグ用に現在実行中のノード名をExecutorに伝える
        if (executorContext)
            executorContext->SetCurrentRunningNode(m_name);
        m_state = BehaviorState::RUNNING;
        m_currentChildIndex = 0;
    }

	// 子ノードを順番に実行
    while (m_currentChildIndex < m_children.size()) {
		// 子ノードを実行
        BehaviorState childState =
            m_children[m_currentChildIndex]->Execute(executorContext);

		// 子ノードがRUNNINGならシーケンスもRUNNING
        if (childState == BehaviorState::RUNNING) {
            return BehaviorState::RUNNING;
        }
		// 子ノードがFAILUREならシーケンスもFAILURE（初期化して次回からやり直し）
        else if (childState == BehaviorState::FAILURE) {
            Initialize();
            return BehaviorState::FAILURE;
        }
		// 子ノードがSUCCESSなら次の子ノードへ
        else
        {
            m_currentChildIndex++;
        }
    }

	// 全ての子ノードがSUCCESSならシーケンスもSUCCESS（初期化して次回からやり直し）
    Initialize();
    return BehaviorState::SUCCESS;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 開始ノードの初期化実行
RootNode::RootNode(const NodeInstance &id, const std::string &name)
    : BehaviorTreeNode(id, name) {}
//

//-----------------------------------------------------------------------------
// 開始ノードの実行
BehaviorState RootNode::Execute(Executor *executorContext) {
  if (executorContext)
    executorContext->SetCurrentRunningNode(m_name);
  if (m_children.empty()) {
    std::cerr << "Error: RootNode has no children to execute." << std::endl;
    return BehaviorState::FAILURE;
  }
  return m_children[0]->Execute(executorContext);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// アクションノードの実行
BehaviorState ActionNode::Execute(Executor *executorContext) {
  if (executorContext)
    executorContext->SetCurrentRunningNode(m_name);

  char dbg[256];
  sprintf_s(dbg, "DEBUG: ActionNode Executing: %s\n", m_name.c_str());
  OutputDebugStringA(dbg);

  // まず自分のアクションを実行
  BehaviorState result = ExecuteAction(executorContext);

  sprintf_s(dbg, "DEBUG: ActionNode %s Result: %d\n", m_name.c_str(),
            (int)result);
  OutputDebugStringA(dbg);

  // SUCCESSで次のノードがあれば実行
  if (result == BehaviorState::SUCCESS) {
    if (m_nextNode) {
      OutputDebugStringA("DEBUG: ActionNode: Moving to Next Node.\n");
      return m_nextNode->Execute(executorContext);
    } else {
      OutputDebugStringA(
          "DEBUG: ActionNode: No Next Node linked! Stopping here.\n");
    }
  }

  return result;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// セレクタノードの初期化実行
SelectorNode::SelectorNode(const NodeInstance &id, const std::string &name)
    : BehaviorTreeNode(id, name) {}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// セレクタノードの初期化実行
void SelectorNode::Initialize() {
  m_state = BehaviorState::READY;
  for (auto &child : m_children) {
    if (child)
      child->Initialize();
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// セレクタノードの実行
BehaviorState SelectorNode::Execute(Executor *executorContext) {
	// READY状態なら実行開始
    if (m_state == BehaviorState::READY) {
        if (executorContext)
            executorContext->SetCurrentRunningNode(m_name);
        m_state = BehaviorState::RUNNING;
        m_currentChildIndex = 0;
    }

  while (m_currentChildIndex < m_children.size()) {
	  // 子ノードを実行
      BehaviorState childState =
          m_children[m_currentChildIndex]->Execute(executorContext);

      // 子ノードがRUNNINGならセレクタもRUNNING
      if (childState == BehaviorState::RUNNING) {
          return BehaviorState::RUNNING;
      }
      // 子ノードがSUCCESSならセレクタもSUCCESS（初期化して次回からやり直し）
      else if (childState == BehaviorState::SUCCESS) {
          Initialize();
          return BehaviorState::SUCCESS;
      }
      // FAILUREなら次の子ノードへ
      else
      {
          m_currentChildIndex++;
      }
  }

  // 全ての子ノードがFAILUREならセレクタもFAILURE（初期化して次回からやり直し）
  Initialize();
  return BehaviorState::FAILURE;
}
//-----------------------------------------------------------------------------