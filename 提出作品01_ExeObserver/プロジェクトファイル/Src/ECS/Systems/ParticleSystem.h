#pragma once
//=============================================================================
// ParticleSystem - ParticleComponent を一括更新・破棄
//   ECS化後のエフェクト（Billboard/Particle）の寿命・移動を管理
//=============================================================================
#include "../World.h"
#include "../../SceneManager.h"

class ParticleSystem : public ISystem
{
public:
    void Update() override
    {
        World& world   = World::GetInstance();
        auto&  pool    = world.Particles();
        auto&  parts   = pool.All();
        float  dt      = SceneManager::DeltaTime();

        for (size_t i = 0; i < parts.size(); )
        {
            ParticleComponent& p = parts[i];

            if (!p.alive)
            {
                ++i;
                continue;
            }

            // 位置更新
            p.position.x += p.velocity.x * dt;
            p.position.y += p.velocity.y * dt;
            p.position.z += p.velocity.z * dt;

            // 火花の落下表現が必要な場合は次行を有効化する（重力加速度・既定は無効）
            // p.velocity.y -= 9.8f * dt;

            // 寿命減算
            p.lifetime -= dt;

            // アルファを寿命に比例させてフェードアウト
            p.alpha = (p.maxLife > 0.0f) ? (p.lifetime / p.maxLife) : 0.0f;
            if (p.alpha < 0.0f) p.alpha = 0.0f;

            // 寿命切れのパーティクルを破棄（swap-and-pop はComponentPool内で実行）
            if (p.lifetime <= 0.0f)
            {
                EntityID id = pool.GetEntity(i);
                world.DestroyEntity(id);
                // DestroyはFlushDestroyQueue()で処理されるため
                // ここでの size() は変化しない → ++i で進む
                ++i;
                continue;
            }

            ++i;
        }
    }
};