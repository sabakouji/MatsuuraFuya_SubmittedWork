#include "BulletSystem.h"
#include "../../SceneManager.h"
#include "../../ObjectManager.h"
#include "../../WeaponBase.h"     // OwnerID（ePC/eENM）
#include "../../EnemyBase.h"
#include "../../Executor.h"
#include "../../MapManager.h"
#include "../../EffectManager.h"
#include "../../Collision.h"

namespace {
    // 弾の攻撃力（旧 WeaponBullet.cpp の AttackPoint を踏襲）
    const int BulletAttackPoint = 50;

    //-------------------------------------------------------------------------
    // owner 別の線分衝突＋ダメージ適用（旧 WeaponBase::HitCheckLine を踏襲）
    //   命中時 true を返し、coll に衝突情報を格納する。
    //-------------------------------------------------------------------------
    bool HitCheckBulletLine(int owner, const VECTOR3& oldPos, const VECTOR3& newPos,
                            MeshCollider::CollInfo* coll)
    {
        if (owner == WeaponBase::ePC)
        {
            // プレイヤーの弾 → 敵群へ判定
            std::list<EnemyBase*> enemys = ObjectManager::FindGameObjects<EnemyBase>();
            for (EnemyBase* enm : enemys)
            {
                if (enm->HitLineToMesh(oldPos, newPos, coll))
                {
                    enm->AddDamage((float)BulletAttackPoint, oldPos);
                    return true;
                }
            }
        }
        else if (owner == WeaponBase::eENM)
        {
            // 敵の弾 → プレイヤー（Executor）へ判定
            Executor* exe = ObjectManager::FindGameObject<Executor>();
            if (exe != nullptr && exe->HitLineToMesh(oldPos, newPos, coll))
            {
                exe->AddDamage((float)BulletAttackPoint, oldPos);
                return true;
            }
        }
        return false;
    }
}

//=============================================================================
// 全弾を一括更新する（移動→衝突→寿命）。
//   破棄は World::DestroyEntity でキューイングし、次フレーム冒頭で実削除される
//   （ParticleSystem と同方式）。
//=============================================================================
void BulletSystem::Update()
{
    World& world   = World::GetInstance();
    auto&  pool    = world.Bullets();
    auto&  bullets = pool.All();
    float  dt      = SceneManager::DeltaTime();

    EffectManager* efm = ObjectManager::FindGameObject<EffectManager>();
    MapManager*    mm  = ObjectManager::FindGameObject<MapManager>();

    for (size_t i = 0; i < bullets.size(); ++i)
    {
        BulletComponent& b = bullets[i];
        if (!b.alive) continue;

        // 位置更新（前位置を衝突の線分始点に使う）
        VECTOR3 oldPos = b.position;
        b.position.x += b.velocity.x * dt;
        b.position.y += b.velocity.y * dt;
        b.position.z += b.velocity.z * dt;

        // 進んだ距離分を残生存距離から減算
        b.lifeDistance -= b.velocity.Length() * dt;

        // 対象（敵/プレイヤー）への線分衝突
        MeshCollider::CollInfo coll;
        if (HitCheckBulletLine(b.owner, oldPos, b.position, &coll))
        {
            if (efm != nullptr) efm->EmitParticleBurst(coll.hitPosition, coll.normal);
            world.DestroyEntity(pool.GetEntity(i));
            continue;
        }

        // マップとの線分衝突
        if (mm != nullptr)
        {
            VECTOR3 hit, normal;
            if (mm->IsCollisionLay(oldPos, b.position, hit, normal))
            {
                if (efm != nullptr) efm->EmitParticleBurst(hit, normal);
                world.DestroyEntity(pool.GetEntity(i));
                continue;
            }
        }

        // 生存距離切れ
        if (b.lifeDistance <= 0.0f)
        {
            world.DestroyEntity(pool.GetEntity(i));
            continue;
        }
    }
}
