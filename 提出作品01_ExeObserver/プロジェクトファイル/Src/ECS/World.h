#pragma once
//=============================================================================
// ECS - World（エンティティ・コンポーネント・システムの統括管理）
//   既存の ObjectManager/GameObject とは共存させ、段階的移行を可能にする
//=============================================================================
#include "Entity.h"
#include "ComponentPool.h"
#include "Components.h"
#include <vector>
#include <unordered_set>
#include <memory>
#include <functional>
#include <cstdint>

//=============================================================================
// ISystem - 全Systemの基底インタフェース
//=============================================================================
class ISystem
{
public:
    virtual ~ISystem() = default;
    virtual void Update() = 0;
};

//=============================================================================
// World クラス
//   - EntityID の生成・破棄
//   - ComponentPool の管理
//   - System の登録・一括実行
//=============================================================================
class World
{
public:
    // シングルトンアクセス
    static World& GetInstance();

    // エンティティを生成してIDを返す
    EntityID CreateEntity();

    // エンティティを破棄（次のUpdateで削除）
    void DestroyEntity(EntityID id);

    // 破棄待ちエンティティを実際に削除（Updateの冒頭で呼ぶ）
    void FlushDestroyQueue();

    // エンティティが有効か確認
    bool IsAlive(EntityID id) const;

    //-------------------------------------------------------------------------
    // コンポーネント操作
    //-------------------------------------------------------------------------
    template<typename T> ComponentPool<T>& GetPool();
    template<typename T> T& AddComponent(EntityID id);
    template<typename T> T* GetComponent(EntityID id);
    template<typename T> bool HasComponent(EntityID id) const;
    template<typename T> void RemoveComponent(EntityID id);

    //-------------------------------------------------------------------------
    // System 管理
    //-------------------------------------------------------------------------
    void RegisterSystem(std::unique_ptr<ISystem> system);
    void UpdateSystems();

    // 全リセット（シーン遷移時）
    void Clear();

    //-------------------------------------------------------------------------
    // ComponentPool 直接アクセス（System内での一括処理用）
    //-------------------------------------------------------------------------
    // 活性: パーティクル＋弾（Bullet）＋敵ボディ（Enemy）
    ComponentPool<ParticleComponent>&       Particles()     { return particles_; }
    ComponentPool<BulletComponent>&         Bullets()       { return bullets_; }
    ComponentPool<EnemyBodyComponent>&      EnemyBodies()   { return enemyBodies_; }

private:
    World() = default;
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    // EntityID 管理
    EntityID                         nextId_       = 0;
    std::unordered_set<EntityID>     alive_;
    std::vector<EntityID>            destroyQueue_;

    // ComponentPools（活性: パーティクル＋弾＋敵ボディ）
    ComponentPool<ParticleComponent>        particles_;
    ComponentPool<BulletComponent>          bullets_;
    ComponentPool<EnemyBodyComponent>       enemyBodies_;

    // Systems
    std::vector<std::unique_ptr<ISystem>>   systems_;
};

//=============================================================================
// テンプレート実装
//=============================================================================
template<typename T> ComponentPool<T>& World::GetPool()
{
    static_assert(sizeof(T) == 0, "未対応のコンポーネント型");
}

// 特殊化マクロ（対応コンポーネントを明示的に登録）
#define WORLD_POOL_SPEC(Type, Member) \
    template<> inline ComponentPool<Type>& World::GetPool<Type>() { return Member; }

WORLD_POOL_SPEC(ParticleComponent,       particles_)
WORLD_POOL_SPEC(BulletComponent,         bullets_)
WORLD_POOL_SPEC(EnemyBodyComponent,      enemyBodies_)

#undef WORLD_POOL_SPEC

template<typename T>
T& World::AddComponent(EntityID id)
{
    return GetPool<T>().Add(id);
}

template<typename T>
T* World::GetComponent(EntityID id)
{
    return GetPool<T>().Get(id);
}

template<typename T>
bool World::HasComponent(EntityID id) const
{
    return const_cast<World*>(this)->GetPool<T>().Has(id);
}

template<typename T>
void World::RemoveComponent(EntityID id)
{
    GetPool<T>().Remove(id);
}