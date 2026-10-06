#pragma once
//=============================================================================
// BulletSystem - BulletComponent を一括更新・破棄
//   旧 OOP WeaponBullet::Update のロジック（移動・線分衝突・ダメージ・寿命）を
//   System へ集約。命中時はヒットスパーク（ECSパーティクル）を発生させる。
//=============================================================================
#include "../World.h"

class BulletSystem : public ISystem
{
public:
    void Update() override;
};
