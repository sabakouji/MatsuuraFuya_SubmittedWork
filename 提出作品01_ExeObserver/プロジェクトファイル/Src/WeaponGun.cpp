#include "EffectManager.h"
#include "EnemyManager.h"
#include "MapManager.h"
#include "WeaponManager.h"

namespace {
const int AttackPoint = 200;
const float BulletWaitTime = 5;
}; // namespace

WeaponGun::WeaponGun() {
  mesh = nullptr;
  meshNo = 0;
  boneNo = 0;
  adjustMatrix = XMMatrixIdentity();
  worldMatrix = XMMatrixIdentity();
  offset = VECTOR3(0, 0, 0);
  bulletTimer = 0;
}

WeaponGun::~WeaponGun() {}

void WeaponGun::SetWeaponGun(std::string name, int meshNoIn, int boneNoIn,
                             VECTOR3 offsetIn, VECTOR3 posIn, VECTOR3 rotIn) {
  if (name == "") {
    mesh = nullptr;
    offset = offsetIn;
  } else {
    mesh = ObjectManager::FindGameObject<WeaponManager>()
               ->MeshList(name)
               ->mesh; // WeaponManager->Mesh List
    offset =
        ObjectManager::FindGameObject<WeaponManager>()->MeshList(name)->offset +
        offsetIn; // WeaponManager Offset + value
  }
  meshNo = meshNoIn;
  boneNo = boneNoIn;
  adjustMatrix = XMMatrixRotationY(rotIn.y * DegToRad) *
                 XMMatrixRotationX(rotIn.x * DegToRad) *
                 XMMatrixRotationZ(rotIn.z * DegToRad);
  adjustMatrix = adjustMatrix * XMMatrixTranslationFromVector(posIn);
}

void WeaponGun::Start() {
  if (Parent() == nullptr)
    DestroyMe();
}

void WeaponGun::Update() {
  EffectManager *efm = ObjectManager::FindGameObject<EffectManager>();

  Object3D *pObj = static_cast<Object3D *>(Parent()); // Parent Object
  if (pObj == nullptr)
    return;

  if (boneNo == -1) {
    worldMatrix = adjustMatrix * pObj->Matrix();
  } else {
    worldMatrix =
        adjustMatrix * pObj->Mesh()->GetFrameMatrices(
                           pObj->GetAnimator(), pObj->Matrix(), boneNo, meshNo);
  }

  transform.position = GetPositionVector(worldMatrix);
  startPos = XMVector3TransformCoord(offset, worldMatrix);

  if (bulletTimer > 0)
    bulletTimer -= 60 * SceneManager::DeltaTime();
}

bool WeaponGun::ShotBullet(WeaponBase::OwnerID owner) {
  bool ret = false;
  if (bulletTimer <= 0) {
    Object3D *pObj = static_cast<Object3D *>(Parent()); // Parent Object
    MATRIX4X4 mat = GetRotateMatrix(pObj->Matrix()) *
                    XMMatrixTranslationFromVector(
                        startPos); // Parent pos, Parent rot Matrix
    VECTOR3 target = XMVector3TransformCoord(VECTOR3(0, 0, 1), mat);
    // ECS弾を生成（旧 OOP WeaponBullet を置換）
    ObjectManager::FindGameObject<WeaponManager>()->EmitBullet(startPos, target,
                                                               owner);
    bulletTimer = BulletWaitTime;
    ret = true;
  }
  return ret;
}

bool WeaponGun::ShotLaser(WeaponBase::OwnerID owner) {
  Object3D *pObj = static_cast<Object3D *>(Parent()); // Parent Object
  WeaponLaser *wl =
      ObjectManager::FindGameObject<WeaponManager>()->Spawn<WeaponLaser>(owner);
  wl->SetPosition(startPos);
  MATRIX4X4 mat =
      GetRotateMatrix(pObj->Matrix()) *
      XMMatrixTranslationFromVector(startPos); // Parent pos, Parent rot Matrix
  wl->SetTarget(XMVector3TransformCoord(VECTOR3(0, 0, 1), mat));
  return true;
}

void WeaponGun::Draw() {
  if (mesh != nullptr) {
    // 所有者に合わせてトゥーンシェーディング（輪郭線付き）で描画する
    if (toonEnabled) {
      mesh->RenderToon(worldMatrix);
    } else {
      mesh->Render(worldMatrix);
    }
  }

  // Helper Line Display
  VECTOR3 end = XMVector3TransformCoord(offset, worldMatrix);
  CSprite spr;
  spr.DrawLine3D(transform.position, end, RGB(255, 0, 0));
}
