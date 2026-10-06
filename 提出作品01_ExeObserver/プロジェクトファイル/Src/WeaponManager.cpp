#include "WeaponManager.h"
#include "AudioManager.h"
#include "SceneManager.h"

namespace {
  const float BulletMoveSpeed   = 2.0f;   // 旧 WeaponBullet の MoveSpeed
  const float BulletLifeDistance = 50.0f; // 旧 WeaponBullet の LifeDistance
}

WeaponManager::WeaponManager() {
  ObjectManager::DontDestroy(this);
  ObjectManager::SetVisible(this, true);  // ECS弾の描画のため Draw を有効化
  mesh = nullptr;
  meshCol = nullptr;

  WeaponBase::meshstruct ms;

  // FireBall - Large
  meshList.push_back(ms);
  meshList.back().name = "FireBall";
  meshList.back().mesh = new CFbxMesh();
  meshList.back().mesh->Load("Data/Item/FireBall.mesh");

  // FireBall2 - Small
  meshList.push_back(ms);
  meshList.back().name = "FireBall2";
  meshList.back().mesh = new CFbxMesh();
  meshList.back().mesh->Load("Data/Item/FireBall2.mesh");

  // Bullet
  meshList.push_back(ms);
  meshList.back().name = "Bullet";
  meshList.back().mesh = new CFbxMesh();
  meshList.back().mesh->Load("Data/Item/Bullet.mesh");

  // Laser
  meshList.push_back(ms);
  meshList.back().name = "Laser";
  meshList.back().mesh = new CFbxMesh();
  meshList.back().mesh->Load("Data/Item/Laser2.mesh");

  // JapanSword
  meshList.push_back(ms);
  meshList.back().name = "JapanSword";
  meshList.back().mesh = new CFbxMesh();
  meshList.back().mesh->Load("Data/Item/Sword.mesh");
  meshList.back().offset = VECTOR3(0, 1.6f, 0);

  // Sword
  meshList.push_back(ms);
  meshList.back().name = "Sword";
  meshList.back().mesh = new CFbxMesh();
  meshList.back().mesh->Load("Data/Item/Weapon_LaserBlade.mesh");
  meshList.back().offset = VECTOR3(0, 1.7f, 0);

  // Rifle
  meshList.push_back(ms);
  meshList.back().name = "Rifle";
  meshList.back().mesh = new CFbxMesh();
  meshList.back().mesh->Load("Data/Item/Gun.mesh");
  meshList.back().offset = VECTOR3(0.51f, 0, 0);

  // Weapon_LaserGun
  meshList.push_back(ms);
  meshList.back().name = "Weapon_LaserGun";
  meshList.back().mesh = new CFbxMesh();
  meshList.back().mesh->Load("Data/Char/Weapon_LaserGun/Weapon_LaserGun.mesh");
  meshList.back().offset = VECTOR3(0, 0, 0);

  // Pistol
  meshList.push_back(ms);
  meshList.back().name = "Pistol";
  meshList.back().mesh = new CFbxMesh();
  meshList.back().mesh->Load("Data/Item/Pistol.mesh");
  meshList.back().offset = VECTOR3(0.2f, 0.1f, 0);

  // ECS弾描画用に共有Bulletメッシュをキャッシュ
  bulletMesh = MeshList("Bullet")->mesh;
}

WeaponManager::~WeaponManager() {
  for (auto &ms : meshList) {
    SAFE_DELETE(ms.mesh);
  }
}

//=============================================================================
// ECS弾を1発生成する（旧 WeaponBullet の生成・初期化を踏襲）。
//   進行方向は startIn→targetIn。BulletSystem が移動・衝突・寿命を一括管理する。
//=============================================================================
void WeaponManager::EmitBullet(VECTOR3 startIn, VECTOR3 targetIn, OwnerID owner) {
  World &world = World::GetInstance();
  EntityID id = world.CreateEntity();
  BulletComponent &b = world.AddComponent<BulletComponent>(id);

  VECTOR3 dir = normalize(targetIn - startIn);  // 進行方向（単位ベクトル）

  b.position     = startIn;
  b.velocity     = dir * (BulletMoveSpeed * 60.0f);  // 秒速（position += vel*dt）
  b.rotation     = GetLookatRotateVector(startIn, targetIn);  // 描画向き
  b.owner        = owner;
  b.lifeDistance = BulletLifeDistance;
  b.alive        = true;

  AudioManager::Audio("EnemyShot")->Play();
}

//=============================================================================
// ECS弾を共有Bulletメッシュで一括描画する（BulletRenderSystem相当）。
//   各 BulletComponent の回転・位置からワールド行列を組み立て Render する。
//=============================================================================
void WeaponManager::Draw() {
  if (bulletMesh == nullptr) return;

  World &world = World::GetInstance();
  auto &bullets = world.Bullets().All();

  for (const BulletComponent &b : bullets) {
    if (!b.alive) continue;

    MATRIX4X4 world4 =
        XMMatrixRotationRollPitchYaw(b.rotation.x, b.rotation.y, b.rotation.z) *
        XMMatrixTranslationFromVector(b.position);
    bulletMesh->Render(world4);
  }
}
