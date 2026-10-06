#include "EnemyPhysicsSystem.h"
#include "../../SceneManager.h"

namespace {
    const float Gravity = 0.025f;  // 重力加速度（正の値・旧 EnemyGolem/EnemyMecha と同値）
}

//=============================================================================
// 全敵ボディの落下速度を重力で一括積分する。
//   speedY は OOP が「接地で0クリア」「位置へ適用」する（EnemyBase::ApplyEnemyGravity）。
//=============================================================================
void EnemyPhysicsSystem::Update()
{
    World& world  = World::GetInstance();
    auto&  bodies = world.EnemyBodies().All();
    float  dt     = SceneManager::DeltaTime();

    for (EnemyBodyComponent& b : bodies)
    {
        if (!b.alive) continue;
        b.speedY -= Gravity * 60.0f * dt;
    }
}
