#include "EnemyCollisionSystem.h"

//=============================================================================
// 全敵ボディの押し合いを算出する。
//   旧 Object3D::HitSphereToSpherePush と同式（XZ平面・押し戻し長 = rsum - 距離）。
//   pushOut は上書きで設定し、適用・0クリアは EnemyBase::ApplyEnemyBodyPush が担う。
//=============================================================================
void EnemyCollisionSystem::Update()
{
    World& world  = World::GetInstance();
    auto&  bodies = world.EnemyBodies().All();
    const size_t n = bodies.size();

    for (size_t i = 0; i < n; ++i)
    {
        EnemyBodyComponent& bi = bodies[i];
        if (!bi.alive) continue;

        VECTOR3 acc = {0.0f, 0.0f, 0.0f};

        for (size_t j = 0; j < n; ++j)
        {
            if (j == i) continue;
            EnemyBodyComponent& bj = bodies[j];
            if (!bj.alive) continue;

            // XZ平面での中心差（旧実装は withY=false で y を無視）
            VECTOR3 d = bi.position - bj.position;
            d.y = 0.0f;

            float rsum = bi.radius + bj.radius;
            float distSq = d.LengthSquare();
            if (distSq < rsum * rsum && distSq > 0.0f)
            {
                float pushLen = rsum - sqrtf(distSq);
                acc += XMVector3Normalize(d) * pushLen;
            }
        }

        bi.pushOut = acc;
    }
}
