#pragma once
#include "Animator.h"
#include "BehaviorTree_Runner.h"
#include "ExecutorContext.h"
#include "NodeDefinition.h"
#include "Object3D.h"
#include "Serializer.h"
#include "WeaponManager.h"
#include <set>
#include <vector>

// TargetBoxType moved to CoreData.h

class EnemyBase;

class Executor : public Object3D {
public:
  Executor();
  ~Executor();

  void Update() override;
  void Draw() override;

  void MoveForward(float speed);
  void MoveTo(const VECTOR3 &targetPos, float speed);
  void MoveStraightTo(const VECTOR3 &targetPos, float speed);
  void StopMove();
  void TurnTo(const VECTOR3 &dir);

  void SetNextAttackSkill(int skillId, float range, float damage, int score);
  void StartAttack(TargetBoxType target);
  bool IsAttacking() const { return m_state == StAttack; }
  int GetCurrentUseSkillScore() const { return m_currentUseSkillScore; }

  bool IsOnGround() const;

  void AddDamage(float damage, const VECTOR3 &fromPos);
  float HpRatio() const { return (float)m_hitPoint / m_maxHitPoint; }

  void AddToTargetBox(TargetBoxType boxType, Object3D *target);

  bool Raycast(const VECTOR3 &start, const VECTOR3 &end,
               MeshCollider::CollInfo *info = nullptr);

  void ClearTargetBox(TargetBoxType boxType);

  void ClearAllTargetBoxes();

  Object3D *GetNearestFromBox(TargetBoxType boxType) const;

  VECTOR3 GetTargetPositionFromBox(TargetBoxType boxType) const;

  bool HasTargetsInBox(TargetBoxType boxType) const;

  int GetTargetCountInBox(TargetBoxType boxType) const;

  const std::vector<Object3D *> &GetTargetBox(TargetBoxType boxType) const;

  void SetManualTargetPosition(const VECTOR3 &pos);
  VECTOR3 GetManualTargetPosition() const { return m_manualTargetPosition; }
  bool HasManualTargetPosition() const { return m_hasManualTargetPosition; }
  void ClearManualTargetPosition();

  float GetCurrentRadarRadius() const { return m_currentRadarRadius; }
  void SetCurrentRadarRadius(float radius) { m_currentRadarRadius = radius; }

  // Ensure targets are still valid (alive)
  void ValidateTargets();

  // Field of View Helpers
  VECTOR3 GetForwardVector() const;
  bool IsTargetInFOV(const Object3D *target, float fovAngleDegrees) const;

  int GetCurrentState() const { return static_cast<int>(m_state); }

  // Behavior Tree Helpers
  void SetCurrentRunningNode(const std::string &name) {
    m_currentRunningNodeName = name;
  }
  std::string GetCurrentRunningNode() const { return m_currentRunningNodeName; }

  std::string GetInputSourceNodeId(const std::string &nodeId,
                                   const std::string &pinName) const;
  const NodeInstance *GetNodeInstance(const std::string &nodeId) const;

private:
  std::shared_ptr<NodeFactory> m_factory;

  WeaponSword *m_swordObj;
  WeaponGun *m_gunObj;

  BehaviorTreeRunner m_btRunner;
  ExecutorContext m_btContext;
  GraphData m_graphData;

  float m_speed;
  VECTOR3 m_velocity;

  int m_hitPoint;
  int m_maxHitPoint;

  std::string m_currentRunningNodeName;

  // Next Attack Parameters
  float m_nextAttackRange;
  float m_nextAttackDamage;
  int m_nextSkillId;
  int m_nextSkillScore;

  std::map<TargetBoxType, std::vector<Object3D *>> m_targetBoxes;
  std::map<TargetBoxType, std::set<Object3D *>>
      m_targetBoxSets; // 重複チェック用

  VECTOR3 m_manualTargetPosition;
  bool m_hasManualTargetPosition;

  float m_currentRadarRadius;

  static inline const std::vector<Object3D *> s_emptyTargetList;

  enum AnimID {
    aIdle = 0,
    aRun = 1,
    aDead,
    aAttack1,
    aAttack2,
    aAttack3,
  };

  enum State {
    stIdle = 0,
    stRun,
    StAttack,
    stDamage,
    stDead,
  };

  enum class AttackPhase {
    phWait = 0,
    phAttack,
    phEnd,
  };

  State m_state;
  State m_prevstate;
  AttackPhase m_attackPhase;

  float m_attackTimer;
  float m_attackRange;
  float m_attackDamage;
  TargetBoxType m_attackTargetBox;
  bool m_hasDealtDamage;
  int m_currentSkillId;
  int m_currentUseSkillScore;

  void UpdateState();
  void ApplyGravityAndMap();
  void UpdateAnimation();
  void UpdateAttack();

  // Navigation
  std::vector<VECTOR3> m_currentPath;
  int m_currentPathIndex;
  VECTOR3 m_currentNavTarget;
  bool m_hasNavTarget;

  void FollowPath(float speed);
};