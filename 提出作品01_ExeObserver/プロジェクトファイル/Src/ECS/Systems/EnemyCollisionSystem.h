#pragma once
//=============================================================================
// EnemyCollisionSystem - 敵同士の押し合いを一括算出
//   旧 OOP の各敵 Update 内 O(N²) 押し戻し（FindGameObjects＋球押し戻し）を
//   EnemyBodyComponent プールの単一パス走査へ集約する。
//   各 body の pushOut を算出し、適用は OOP（EnemyBase::ApplyEnemyBodyPush）が行う。
//=============================================================================
#include "../World.h"

class EnemyCollisionSystem : public ISystem
{
public:
    void Update() override;
};
