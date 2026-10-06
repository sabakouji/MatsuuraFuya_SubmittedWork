#include "EnemyMecha.h"
#include "BBox.h"
#include "DataCarrier.h"
#include "EnemyManager.h"
#include "GameMain.h"
#include "MyMath.h"
#include "SceneManager.h"
#include "WeaponManager.h"

namespace {
const float PatrolMoveSpeed = 0.05f;
const float ChaseMoveSpeed = 0.05f;
const float RotationSpeed = 3.0f;
const float DetectionRange = 30.0f;  // 索敵範囲
const float ShootRange = 25.0f;      // 射撃範囲
const float KickRange = 2.0f;        // キック範囲
const float KickDamage = 50.0f;      // キックダメージ
const float ShootInterval = 60.0f;   // 1秒 (60fps)
const float KickCooldown = 60.0f;    // キック後1秒待機
const float PatrolWaitTime = 180.0f; // パトロール地点での待機時間
const int MaxHitPoint = 500;         // This will be overridden by m_maxHitPoint
} // namespace

EnemyMecha::EnemyMecha() {
  animator = new Animator();

  // EnemyManagerからメッシュを取得 (EnemyManagerの実装に合わせてキーを指定)
  mesh = ObjectManager::FindGameObject<EnemyManager>()->MeshList("EnemyMecha");
  animator->SetModel(mesh);
  animator->Play(aWalk); // 初期は歩行にしておく（パトロール想定）
  animator->SetPlaySpeed(1.0f);

  // トゥーンシェーディングと輪郭線を有効にする
  SetToonEnabled(true);
  SetOutline(true, VECTOR4(0.0f, 0.0f, 0.0f, 1.0f), 2.0f);

  meshCol = new MeshCollider();
  meshCol->MakeFromMesh(mesh, animator);

  state = stNormal;
  m_maxHitPoint = 1000; // HP
  hitPoint = m_maxHitPoint;

  isChasing = false;
  patrolIndex = 0;
  patrolWaitTimer = 0.0f;
  targetPlayer = nullptr;

  shootTimer = 0.0f; // Changed from ShootInterval
  isKicking = false;
  kickCooldownTimer = 0.0f;
  hasDealtDamage = false; // New member variable

  // Initialize targetRotation
  targetRotation = transform.rotation;

  debugKickBoxVisible = false;

  debugKickBox =
      new CBBox(GameDevice()->m_pShader, VECTOR3(-2.0f, -3.0f, -3.0f),
                VECTOR3(2.0f, 3.0f, 3.0f));
  debugKickBox->m_vDiffuse =
      VECTOR4(1.0f, 0.0f, 0.0f, 0.5f); // Red, translucent

  RegisterEnemyBody(3.0f); // ECS: 敵ボディを登録（押し合い半径=Colliderと同値）
}

EnemyMecha::~EnemyMecha() {
  UnregisterEnemyBody();     // ECS: 敵ボディを解除
  SAFE_DELETE(debugKickBox); // Added for debugKickBox
  SAFE_DELETE(meshCol);
}

SphereCollider EnemyMecha::Collider() {
  SphereCollider col;
  col.radius = 3.0f;
  col.center = transform.position + VECTOR3(0, col.radius, 0);
  return col;
}

void EnemyMecha::Update() {
  SyncEnemyBodyIn();  // ECS: 現在位置をコンポーネントへ同期

  VECTOR3 positionOld = transform.position;

  // ダメージ点滅処理
  if (state == stFlash) {
    flashTimer -= 60 * SceneManager::DeltaTime();
    if (flashTimer <= 0)
      state = stNormal;
  } else if (state == stDamage) {
    // ダメージモーションがないため、即座に点滅状態へ移行
    state = stFlash;
    flashTimer = 15.0f; // 0.25秒点滅
  } else if (state == stDead) {
    if (animator->PlayingID() != aDead) {
      animator->Play(aDead);
    }
    if (animator->Finished()) {
      DataCarrier *dc = ObjectManager::FindGameObject<DataCarrier>();
      dc->AddScore(200);
      DestroyMe();
    }
    animator->Update();
    return;
  }

  // 敵同士の押し合い（ECS: EnemyCollisionSystem が算出した押し戻しを適用）
  ApplyEnemyBodyPush();

  // 重力・マップ接触（ECS: EnemyPhysicsSystemが積分した落下速度をOOPが適用）
  ApplyEnemyGravity(positionOld);

  // メインロジック
  if (isChasing) {
    UpdateChase();
  } else {
    UpdatePatrol();
    // 索敵
    Executor *executor = ObjectManager::FindGameObject<Executor>();
    if (CheckReach(executor, 360.0f, DetectionRange)) { // 360度視界で簡易判定
      isChasing = true;
      targetPlayer = executor;
    }
  }

  animator->Update();
}

void EnemyMecha::UpdatePatrol() {
  if (navigationMap.empty())
    return;

  VECTOR3 target = navigationMap[patrolIndex];

  // 到達判定のために距離を確認
  VECTOR3 toTarget = target - transform.position;
  toTarget.y = 0; // 高さは無視

  if (patrolWaitTimer > 0) {
    patrolWaitTimer -= 60 * SceneManager::DeltaTime();
    animator->Play(aIdle);
    if (patrolWaitTimer <= 0) {
      patrolIndex = (patrolIndex + 1) % navigationMap.size();
    }
  } else {
    animator->Play(aWalk);
    if (MoveToTarget(target, PatrolMoveSpeed, RotationSpeed)) {
      patrolWaitTimer = PatrolWaitTime;
    }
  }
}

void EnemyMecha::UpdateChase() {
  if (!targetPlayer) {
    isChasing = false;
    return;
  }

  float dist = magnitude(targetPlayer->Position() - transform.position);
  // キック状態の処理
  if (isKicking) {
    // 攻撃判定フレーム (例: 15〜25フレーム)
    if (!hasDealtDamage && animator->CurrentFrame() >= 15.0f &&
        animator->CurrentFrame() <= 25.0f) {
      if (targetPlayer) {
        // ボックス判定 (自前実装)
        // ターゲットの位置を自分のローカル座標系(Y軸回転のみ考慮)に変換して判定する
        VECTOR3 targetPos = targetPlayer->Collider().center;
        VECTOR3 diff = targetPos - transform.position;

        // Y軸の逆回転をかけてローカルＸＺ座標を求める
        // rotation.y はラジアン
        float r = -transform.rotation.y;
        float c = cosf(r);
        float s = sinf(r);

        // ローカル座標 (X:左右, Z:前後)
        float localX = diff.x * c - diff.z * s;
        float localZ = diff.x * s + diff.z * c;

        // 判定範囲定義
        // 前方: 0.0f 〜 KickRange (6.0f)
        // 横幅: -2.0f 〜 2.0f (幅4m)
        // 高さ: 上下 3.0f 以内
        const float BoxWidthHalf = 2.0f;
        const float BoxHeightHalf = 3.0f;

        // デバッグボックスのワールド行列更新
        if (debugKickBoxVisible) {
          if (debugKickBox) {
            MATRIX4X4 boxOffset = XMMatrixTranslation(0, 2.0f, 3.0f);
            MATRIX4X4 rotM = XMMatrixRotationRollPitchYaw(transform.rotation.x,
                                                          transform.rotation.y,
                                                          transform.rotation.z);
            debugKickBox->m_mWorld =
                boxOffset * rotM *
                XMMatrixTranslation(transform.position.x, transform.position.y,
                                    transform.position.z);
          }
        }

        if (localZ >= 0.0f && localZ <= KickRange &&
            fabs(localX) <= BoxWidthHalf && fabs(diff.y) <= BoxHeightHalf) {
          // ダメージを与える
          targetPlayer->AddDamage(KickDamage, transform.position);
          hasDealtDamage = true;
        }
      }
    }

    if (animator->Finished()) {
      isKicking = false;
      kickCooldownTimer = KickCooldown;
      animator->Play(aWalk); // 戻る
    }
    return; // キック中は他の行動をしない
  }

  // クールダウン消化
  if (kickCooldownTimer > 0) {
    kickCooldownTimer -= 60 * SceneManager::DeltaTime();
  }

  // プレイヤーの方を向く
  targetRotation =
      GetLookatRotateVector(transform.position, targetPlayer->Position());

  // 行動分岐
  // 1. キック範囲内ならキック (クールダウン完了時)
  if (dist <= KickRange && kickCooldownTimer <= 0) {
    PerformKick();
    return;
  }

  // 2. 射撃 (キック中でなければ)
  // 追跡中で射程内
  if (dist <= ShootRange) {
    shootTimer -= 60 * SceneManager::DeltaTime();
    if (shootTimer <= 0) {
      PerformShoot();
      shootTimer = ShootInterval;
    }
  }

  // 3. 移動制御
  // キック範囲より遠ければ近づく
  if (dist >
      KickRange - 1.0f) { // 重なりすぎないように少し手前ターゲットも考慮など
    MoveToTarget(targetPlayer->Position(), ChaseMoveSpeed, RotationSpeed);
    animator->Play(aWalk);
  } else {
    // 近すぎる場合は待機または周り込み（今回は待機/Idle）
    animator->Play(aIdle);
    // 回転だけ合わせる

    // 補間処理:
    VECTOR3 diff = targetRotation - transform.rotation;
    // 正規化 (-PI ~ PI)
    while (diff.y > XM_PI)
      diff.y -= XM_2PI;
    while (diff.y < -XM_PI)
      diff.y += XM_2PI;

    if (fabs(diff.y) > RotationSpeed * 0.01f) {
      if (diff.y > 0)
        transform.rotation.y += RotationSpeed * 0.01f;
      else
        transform.rotation.y -= RotationSpeed * 0.01f;
    }
  }

  // 追跡解除判定（例：遠すぎたら諦める）
  if (dist > DetectionRange * 1.5f) {
    isChasing = false;
    targetPlayer = nullptr;
    patrolWaitTimer = 0; // すぐパトロール再開
  }
}

void EnemyMecha::PerformShoot() {
  if (!targetPlayer)
    return;

  VECTOR3 targetPos = targetPlayer->Collider().center;

  MATRIX4X4 rotM = XMMatrixRotationRollPitchYaw(
      transform.rotation.x, transform.rotation.y, transform.rotation.z);

  WeaponManager *wm = ObjectManager::FindGameObject<WeaponManager>();

  // 左腕（ECS弾を生成）
  VECTOR3 offsetLeft =
      XMVector3TransformCoord(VECTOR3(-1.5f, 2.0f, 1.0f), rotM);
  wm->EmitBullet(transform.position + offsetLeft, targetPos, WeaponBase::eENM);

  // 右腕（ECS弾を生成）
  VECTOR3 offsetRight =
      XMVector3TransformCoord(VECTOR3(1.5f, 2.0f, 1.0f), rotM);
  wm->EmitBullet(transform.position + offsetRight, targetPos, WeaponBase::eENM);
}

void EnemyMecha::PerformKick() {
  isKicking = true;
  hasDealtDamage = false;
  animator->Play(aAttack1);
}

// -----------------------------------------------------------------------------------------
// 描画
// -----------------------------------------------------------------------------------------
void EnemyMecha::Draw() {
  EnemyBase::Draw();
}
