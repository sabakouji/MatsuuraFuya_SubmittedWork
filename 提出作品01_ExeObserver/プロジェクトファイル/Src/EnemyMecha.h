#pragma once
#include "Animator.h"
#include "EnemyBase.h"
#include "Executor.h"

class EnemyMecha : public EnemyBase {
public:
  EnemyMecha();
  virtual ~EnemyMecha();

  void Update() override;
  void Draw() override;
  SphereCollider Collider() override;

private:
  void UpdatePatrol();
  void UpdateChase();

  void PerformShoot();
  void PerformKick();

  bool isChasing;
  int patrolIndex;
  float patrolWaitTimer;

  float shootTimer;
  bool isKicking;
  float kickCooldownTimer;

  Executor *targetPlayer;

  VECTOR3 targetRotation;

  bool hasDealtDamage;
  int m_maxHitPoint;

  class CBBox *debugKickBox;
  bool debugKickBoxVisible;

  // Animation IDs (Local definition matching what will be loaded)
  enum AnimID { aIdle = 0, aWalk = 1, aDead = 2, aKick = 3 };
};
