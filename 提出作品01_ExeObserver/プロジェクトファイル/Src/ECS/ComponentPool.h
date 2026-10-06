#pragma once
//=============================================================================
// ECS - ComponentPool（packed array による連続メモリ管理）
//   ※同種コンポーネントを密に連続配置する packed array 方式（厳密なSoAではない）
//   同種のコンポーネントを std::vector で連続管理し
//   キャッシュフレンドリーな一括処理を可能にする
//=============================================================================
#include "Entity.h"
#include <vector>
#include <unordered_map>
#include <cassert>

template<typename T>
class ComponentPool
{
public:
    // コンポーネントを追加して参照を返す
    T& Add(EntityID id)
    {
        assert(entityToIndex_.find(id) == entityToIndex_.end() && "既にコンポーネントが存在する");
        size_t index = data_.size();
        data_.emplace_back(T{});
        entityToIndex_[id] = index;
        indexToEntity_.push_back(id);
        return data_[index];
    }

    // コンポーネントを取得（存在しない場合はnullptrを返す）
    T* Get(EntityID id)
    {
        auto it = entityToIndex_.find(id);
        if (it == entityToIndex_.end()) return nullptr;
        return &data_[it->second];
    }

    const T* Get(EntityID id) const
    {
        auto it = entityToIndex_.find(id);
        if (it == entityToIndex_.end()) return nullptr;
        return &data_[it->second];
    }

    // コンポーネントが存在するか確認
    bool Has(EntityID id) const
    {
        return entityToIndex_.find(id) != entityToIndex_.end();
    }

    // コンポーネントを削除（swap-and-pop でO(1)削除）
    void Remove(EntityID id)
    {
        auto it = entityToIndex_.find(id);
        if (it == entityToIndex_.end()) return;

        size_t removeIdx  = it->second;
        size_t lastIdx    = data_.size() - 1;

        if (removeIdx != lastIdx)
        {
            // 末尾要素を削除位置に移動
            data_[removeIdx]                     = std::move(data_[lastIdx]);
            EntityID movedEntity                 = indexToEntity_[lastIdx];
            entityToIndex_[movedEntity]          = removeIdx;
            indexToEntity_[removeIdx]            = movedEntity;
        }

        data_.pop_back();
        indexToEntity_.pop_back();
        entityToIndex_.erase(it);
    }

    // 全コンポーネントへの連続アクセス（Systemの一括処理用）
    std::vector<T>& All()             { return data_; }
    const std::vector<T>& All() const { return data_; }

    // エンティティIDの逆引き（インデックス → EntityID）
    EntityID GetEntity(size_t index) const
    {
        assert(index < indexToEntity_.size());
        return indexToEntity_[index];
    }

    // 管理中のコンポーネント数
    size_t Size() const { return data_.size(); }

    // 全削除
    void Clear()
    {
        data_.clear();
        entityToIndex_.clear();
        indexToEntity_.clear();
    }

private:
    std::vector<T>                        data_;           // 連続メモリ（packed array）
    std::unordered_map<EntityID, size_t>  entityToIndex_;  // EntityID → インデックス
    std::vector<EntityID>                 indexToEntity_;  // インデックス → EntityID
};