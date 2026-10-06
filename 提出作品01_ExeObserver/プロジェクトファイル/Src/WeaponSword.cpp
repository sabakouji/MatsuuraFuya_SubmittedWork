#include "WeaponSword.h"
#include "EffectManager.h"
#include "EnemyManager.h"
#include "MapManager.h"
#include "Player.h"
#include "WeaponManager.h"
#include <string>

namespace {
const int AttackPoint = 200;
};

WeaponSword::WeaponSword() {
  mesh = nullptr;
  meshNo = 0;
  boneNo = 0;
  adjustMatrix = XMMatrixIdentity();
  worldMatrix = XMMatrixIdentity();
  offset = VECTOR3(0, 0, 0);
  m_collisionEnabled = false;
}

WeaponSword::~WeaponSword() {}

void WeaponSword::SetWeaponSword(std::string name, int meshNoIn, int boneNoIn,
                                 VECTOR3 offsetIn, VECTOR3 posIn,
                                 VECTOR3 rotIn) {
  if (name.empty()) {
    mesh = nullptr;
    offset = offsetIn;
  } else {
    WeaponManager *wm = ObjectManager::FindGameObject<WeaponManager>();
    if (wm) {
      auto *meshItem = wm->MeshList(name);
      if (meshItem) {
        mesh = meshItem->mesh;
        offset = meshItem->offset + offsetIn;
      }
    }
  }
  meshNo = meshNoIn;
  boneNo = boneNoIn;

  adjustMatrix = XMMatrixRotationY(rotIn.y * DegToRad) *
                 XMMatrixRotationX(rotIn.x * DegToRad) *
                 XMMatrixRotationZ(rotIn.z * DegToRad);
  adjustMatrix = adjustMatrix * XMMatrixTranslationFromVector(posIn);
}

void WeaponSword::Start() {
  if (Parent() == nullptr) {
    DestroyMe();
  }
}

void WeaponSword::Update() {
  EffectManager *efm = ObjectManager::FindGameObject<EffectManager>();

  Object3D *pObj = static_cast<Object3D *>(Parent());
  if (pObj == nullptr)
    return;

  worldMatrix =
      adjustMatrix *
      pObj->Mesh()->GetFrameMatrices(pObj->GetAnimator(), boneNo, meshNo) *
      pObj->Matrix();
  transform.position = GetPositionVector(worldMatrix);
  VECTOR3 end = XMVector3TransformCoord(offset, worldMatrix);

  MeshCollider::CollInfo coll;
  bool hit = false;

  if (m_collisionEnabled) {
    hit = HitCheckLine(transform.position, end, &coll, AttackPoint);
    if (hit && efm) {
      efm->EmitParticleBurst(coll.hitPosition, coll.normal);
    }
  }

  if (!hit) {
    MapManager *mm = ObjectManager::FindGameObject<MapManager>();
    if (mm != nullptr) {
      VECTOR3 hitPos, normal;
      if (mm->IsCollisionLay(transform.position, end, hitPos, normal)) {
        if (efm)
          efm->EmitParticleBurst(hitPos, normal);
      }
    }
  }
}

void WeaponSword::Draw() {
  if (mesh != nullptr) {
    // 所有者に合わせてトゥーンシェーディング（輪郭線付き）で描画する
    if (toonEnabled) {
      mesh->RenderToon(worldMatrix);
    } else {
      mesh->Render(worldMatrix);
    }
  }

  VECTOR3 end = XMVector3TransformCoord(offset, worldMatrix);
  CSprite spr;
  spr.DrawLine3D(transform.position, end, RGB(255, 0, 0));
}
