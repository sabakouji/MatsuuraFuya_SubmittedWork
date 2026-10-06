#pragma once
#include "BehaviorTree_Nodes.h"
#include "CoreData.h" // For TargetBoxType
#include <string>
#include <vector>

class Executor;

class EnemyBase;
class Object3D;

//=============================================================================
// 移動ノード
//=============================================================================
class RunAction : public ActionNode {
public:
  RunAction(const NodeInstance &nodeId, float speed = 1.0f,
            float arrivalDistance = 1.0f,
            TargetBoxType targetBox = TargetBoxType::None,
            const std::string &name = "Run")
      : ActionNode(nodeId, name), m_speed(speed),
        m_arrivalDistance(arrivalDistance), m_targetBox(targetBox) {}

  BehaviorState ExecuteAction(Executor *executorContext) override;

private:
  float m_speed;
  float m_arrivalDistance;
  TargetBoxType m_targetBox;
};

class StopAction : public ActionNode {
public:
  StopAction(const NodeInstance &id, const std::string &name = "Stop")
      : ActionNode(id, name) {}

  BehaviorState ExecuteAction(Executor *executorContext) override;
};

//=============================================================================
// RadarScanAction - 拡張式レーダーノード
// 徐々に半径を広げながらスキャンを実行し、種類別のボックスに分類
//=============================================================================
class RadarScanAction : public ActionNode {
public:
  RadarScanAction(const NodeInstance &id, float maxRadius, float expandSpeed,
                  float scanInterval, const std::string &name = "RadarScan")
      : ActionNode(id, name), m_maxRadius(maxRadius),
        m_expandSpeed(expandSpeed), m_scanInterval(scanInterval),
        m_currentRadius(0.0f), m_elapsedTime(0.0f), m_isScanning(false) {}

  void Initialize() override;
  BehaviorState ExecuteAction(Executor *executorContext) override;

private:
  float m_maxRadius;
  float m_expandSpeed;
  float m_scanInterval;
  float m_currentRadius;
  float m_elapsedTime;
  bool m_isScanning;

  void ScanObjectsInRadius(Executor *executorContext);
  TargetBoxType ClassifyObject(Object3D *obj) const;
};

//=============================================================================
// InTheEyesCondition - 視野判定
// 指定したTargetBox内のオブジェクトが視野角内に存在するかチェック
//=============================================================================
class InTheEyesCondition : public ConditionNode {
public:
  InTheEyesCondition(const NodeInstance &id, TargetBoxType boxType, float angle,
                     const std::string &name = "InTheEyes")
      : ConditionNode(id, name), m_boxType(boxType), m_angle(angle) {}

  BehaviorState Execute(Executor *executorContext) override;

private:
  TargetBoxType m_boxType;
  float m_angle;
};

//=============================================================================
// RadarPulseAction - 統合パルス式レーダーノード
// 即座にスキャンを実行し、種類別のボックスに分類
//=============================================================================
class RadarPulseAction : public ActionNode {
public:
  RadarPulseAction(const NodeInstance &id, float radius,
                   const std::string &name = "RadarPulse")
      : ActionNode(id, name), m_radius(radius), m_hasScanned(false) {}

  void Initialize() override;
  BehaviorState ExecuteAction(Executor *executorContext) override;

private:
  float m_radius;
  bool m_hasScanned;

  TargetBoxType ClassifyObject(Object3D *obj) const;
};

//=============================================================================
// SetTargetPositionAction - Positionボックスに座標を設定
//=============================================================================
class SetTargetPositionAction : public ActionNode {
public:
  SetTargetPositionAction(const NodeInstance &id, float x, float y, float z,
                          const std::string &name = "SetTargetPosition")
      : ActionNode(id, name), m_x(x), m_y(y), m_z(z) {}

  BehaviorState ExecuteAction(Executor *executorContext) override;

private:
  float m_x, m_y, m_z;
};

//=============================================================================
// ClearTargetBoxAction - 指定したボックスをクリア
//=============================================================================
class ClearTargetBoxAction : public ActionNode {
public:
  ClearTargetBoxAction(const NodeInstance &id,
                       TargetBoxType boxType = TargetBoxType::All,
                       const std::string &name = "ClearTargetBox")
      : ActionNode(id, name), m_boxType(boxType) {}

  BehaviorState ExecuteAction(Executor *executorContext) override;

private:
  TargetBoxType m_boxType;
};

//=============================================================================
// 条件ノード
//=============================================================================

// HasTargetsInBoxCondition - 指定したボックスにターゲットが存在するか確認
class HasTargetsInBoxCondition : public ActionNode {
public:
  HasTargetsInBoxCondition(const NodeInstance &id,
                           TargetBoxType boxType = TargetBoxType::All,
                           int minTargets = 1,
                           const std::string &name = "HasTargetsInBox")
      : ActionNode(id, name), m_boxType(boxType), m_minTargets(minTargets) {}

  BehaviorState ExecuteAction(Executor *executorContext) override;

private:
  TargetBoxType m_boxType;
  int m_minTargets;
};

// IsNearTargetCondition - 指定したボックスの最も近いターゲットに近いか確認
class IsNearTargetCondition : public ActionNode {
public:
  IsNearTargetCondition(const NodeInstance &id,
                        TargetBoxType boxType = TargetBoxType::All,
                        float distance = 2.0f,
                        const std::string &name = "IsNearTarget")
      : ActionNode(id, name), m_boxType(boxType), m_distance(distance) {}

  BehaviorState ExecuteAction(Executor *executorContext) override;

private:
  TargetBoxType m_boxType;
  float m_distance;
};

// CheckAttackRangeCondition - 攻撃範囲内か確認
class CheckAttackRangeCondition : public BehaviorTreeNode {
public:
  CheckAttackRangeCondition(const NodeInstance &id, TargetBoxType targetBox,
                            const std::string &name = "CheckAttackRange")
      : BehaviorTreeNode(id, name), m_targetBox(targetBox), m_trueNode(nullptr),
        m_falseNode(nullptr) {}

  void SetTrueNode(Nodeptr node) { m_trueNode = std::move(node); }
  void SetFalseNode(Nodeptr node) { m_falseNode = std::move(node); }

  BehaviorTreeNode *GetTrueNode() const { return m_trueNode.get(); }
  BehaviorTreeNode *GetFalseNode() const { return m_falseNode.get(); }

  BehaviorState Execute(Executor *executorContext) override;

private:
  TargetBoxType m_targetBox;
  Nodeptr m_trueNode;
  Nodeptr m_falseNode;
};

//=============================================================================
// ExecuteAttackAction - 攻撃を実行
//=============================================================================
class ExecuteAttackAction : public ActionNode {
public:
  ExecuteAttackAction(const NodeInstance &id, TargetBoxType targetBox,
                      const std::string &name = "ExecuteAttack")
      : ActionNode(id, name), m_targetBox(targetBox), m_isAttacking(false) {}

  void Initialize() override;
  BehaviorState ExecuteAction(Executor *executorContext) override;

private:
  TargetBoxType m_targetBox;
  bool m_isAttacking;
  float m_attackTimer;

  static constexpr float ATTACK_DURATION = 1.0f;
};

//=============================================================================
// FindRouteToGoalNode
//=============================================================================
class FindRouteToGoalNode : public ActionNode {
public:
  FindRouteToGoalNode(const NodeInstance &id,
                      const std::string &name = "FindRouteToGoal")
      : ActionNode(id, name) {}

  BehaviorState ExecuteAction(Executor *executorContext) override;
};

//=============================================================================
// DetectObstacleNode
//=============================================================================
class DetectObstacleNode : public ActionNode {
public:
  DetectObstacleNode(const NodeInstance &id, float checkDistance,
                     float checkRadius,
                     const std::string &name = "DetectObstacle")
      : ActionNode(id, name), m_checkDistance(checkDistance),
        m_checkRadius(checkRadius) {}

  void Initialize() override;
  BehaviorState ExecuteAction(Executor *executorContext) override;

private:
  float m_checkDistance;
  float m_checkRadius;
};

//=============================================================================
// AttackSkillNode
//=============================================================================
class AttackSkillNode : public ActionNode {
public:
  AttackSkillNode(const NodeInstance &id, float range, float damage,
                  float cooldown, int skillId, int score,
                  const std::string &name)
      : ActionNode(id, name), m_range(range), m_damage(damage),
        m_cooldown(cooldown), m_skillId(skillId), m_score(score) {}

  BehaviorState ExecuteAction(Executor *executorContext) override;

protected:
  float m_range;
  float m_damage;
  float m_cooldown;
  int m_skillId;
  int m_score;
};

//=============================================================================
// Skill_FireballNode
//=============================================================================
class Skill_FireballNode : public AttackSkillNode {
public:
  Skill_FireballNode(const NodeInstance &id,
                     const std::string &name = "Skill_Fireball")
      : AttackSkillNode(id, 15.0f, 20.0f, 2.0f, 1, 500,
                        name) // ID 1 for Fireball
  {}
};

//=============================================================================
// Skill_SwordSlashNode
//=============================================================================
class Skill_SwordSlashNode : public AttackSkillNode {
public:
  Skill_SwordSlashNode(const NodeInstance &id,
                       const std::string &name = "Skill_SwordSlash")
      : AttackSkillNode(id, 2.0f, 30.0f, 1.0f, 2, 300,
                        name) // ID 2 for SwordSlash
  {}
};

//=============================================================================
// GetStageTaskNode
//=============================================================================
class GetStageTaskNode : public ActionNode {
public:
  GetStageTaskNode(const NodeInstance &id,
                   const std::string &name = "GetStageTask")
      : ActionNode(id, name) {}

  BehaviorState ExecuteAction(Executor *executorContext) override;
};