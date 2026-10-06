#pragma once
#include "ECS/World.h"
#include "Animator.h"

#include "WeaponBase.h"
#include <list>
#include <string>

#include "WeaponFireBall.h"
#include "WeaponFireBall2.h"
#include "WeaponGun.h"
#include "WeaponLaser.h"
#include "WeaponSword.h"

class WeaponManager : public WeaponBase {
public:
  WeaponManager();
  ~WeaponManager();

  // ECS弾の描画（World::Bullets() を共有Bulletメッシュで一括Render）
  void Draw() override;

  // ECS弾を1発生成する（旧 WeaponBullet 生成を置換）
  //   startIn : 発射位置 / targetIn : 狙い位置（進行方向の決定に使用）
  //   owner   : 所有者区分（ePC=プレイヤー弾 / eENM=敵弾）
  void EmitBullet(VECTOR3 startIn, VECTOR3 targetIn, OwnerID owner);

  // Spawn weapon
  template <class C> C *Spawn(OwnerID owner) {
    C *obj = Instantiate<C>();
    if (obj == nullptr)
      return nullptr;
    obj->SetOwner(owner);
    return obj;
  }

  // Spawn weapon with target
  template <class C>
  C *Spawn(VECTOR3 startIn, VECTOR3 targetIn, OwnerID owner) {
    C *obj = Instantiate<C>();
    if (obj == nullptr)
      return nullptr;
    obj->SetPosition(startIn);
    obj->SetTarget(targetIn);
    obj->SetOwner(owner);
    return obj;
  }

  // Spawn many weapons
  template <class C>
  bool SpawnMany(VECTOR3 startIn, VECTOR3 targetIn, OwnerID owner, int num = 5,
                 float offset = 1.0f) {
    for (int i = 0; i < num; i++) {
      C *obj = Instantiate<C>();
      if (obj == nullptr)
        return false;
      obj->SetPosition(startIn +
                       VECTOR3(Randomf(-offset * 0.5f, offset * 0.5f),
                               Randomf(-offset * 0.5f, offset * 0.5f),
                               Randomf(-offset * 0.5f, offset * 0.5f)));
      obj->SetTarget(targetIn +
                     VECTOR3(Randomf(-offset * 0.8f, offset * 0.8f),
                             Randomf(-offset * 0.8f, offset * 0.8f),
                             Randomf(-offset * 0.8f, offset * 0.8f)));
      obj->SetOwner(owner);
    }
    return true;
  }

  WeaponBase::meshstruct *MeshList(std::string str) {
    for (auto &ms : meshList) {
      if (str == ms.name)
        return &ms;
    }
    std::string msg = "Mesh not found: " + str;
    MessageBox(nullptr, msg.c_str(), "WeaponManager::MeshList()", MB_OK);
    return nullptr;
  }

private:
  std::list<WeaponBase::meshstruct> meshList;
  CFbxMesh* bulletMesh = nullptr;  // 共有Bulletメッシュ（ECS弾描画用キャッシュ）
};