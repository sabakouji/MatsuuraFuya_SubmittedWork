#pragma once
//=============================================================================
// EnemyPhysicsSystem - 敵の落下速度（重力）を一括積分
//   旧 OOP の各敵 Update 内 `speedY -= Gravity*60*dt` を
//   EnemyBodyComponent プールの単一パスへ集約する。
//   速度の適用・接地時の0クリアは OOP（EnemyBase::ApplyEnemyGravity）が行う。
//=============================================================================
#include "../World.h"

class EnemyPhysicsSystem : public ISystem
{
public:
    void Update() override;
};
