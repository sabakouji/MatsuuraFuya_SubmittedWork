#include "Executor.h"
#include "EnemyBase.h"
#include "MapBase.h"
#include "MapManager.h"
#include "MeshCollider.h"
#include "NavigationManager.h"
#include "ObjectManager.h"
#include "SceneManager.h"
#include <cmath>
#include <iostream>
#include <typeinfo>

//-----------------------------------------------------------------------------
// 定数
namespace {
static constexpr float gravity = 0.025f;
static constexpr float moveSpeedBase = 0.05f;
static constexpr float ATTACK_DURATION = 1.0f;
static constexpr float ATTACK_HIT_TIME = 0.4f;
static constexpr float ATTACK_HIT_FRAME = 25.0f;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 初期化
Executor::Executor()
    : m_btRunner(), m_manualTargetPosition(0.0f, 0.0f, 0.0f),
      m_hasManualTargetPosition(false), m_currentRadarRadius(0.0f),
      m_state(State::stIdle), m_prevstate(State::stIdle), m_attackTimer(0.0f),
      m_attackRange(0.0f), m_attackDamage(0.0f),
      m_attackTargetBox(TargetBoxType::None), m_currentSkillId(0),
      m_hasDealtDamage(false), m_nextAttackRange(1.0f),
      m_nextAttackDamage(10.0f), m_nextSkillId(0), m_nextSkillScore(0),
      m_currentUseSkillScore(0) {
    animator = new Animator();

    mesh = new CFbxMesh();

    mesh->Load("Data/Char/Executor/Executor.mesh");
    mesh->LoadAnimation(aIdle, "Data/Char/Executor/MechaGirl_Idel.anmx", true);
    mesh->LoadAnimation(aRun, "Data/Char/Executor/Executor_Run.anmx", true);
    mesh->LoadAnimation(aDead, "Data/Char/Executor/MechaGirl_Dying.anmx", false);
    mesh->LoadAnimation(aAttack1, "Data/Char/Executor/MechaGirl_Slash.anmx",
        false);
    mesh->LoadAnimation(aAttack2, "Data/Char/Executor/MechaGirl_Slash.anmx",
        false);
    mesh->LoadAnimation(aAttack3, "Data/Char/Executor/MechaGirl_Slash.anmx",
        false);

    WeaponManager* wm = ObjectManager::FindGameObject<WeaponManager>();
    m_swordObj = wm->Spawn<WeaponSword>(WeaponBase::ePC);
    m_swordObj->SetWeaponSword("Sword", 0, 19, VECTOR3(0, 0, 0),
        VECTOR3(-0.1f, 0.0f, 0.05f),
        VECTOR3(20.0f, 90.0f, -170.0f));
    m_swordObj->SetParent(this);
    m_swordObj->SetOwner(WeaponBase::ePC);
    m_swordObj->SetActive(true);
    m_swordObj->SetToonEnabled(true);                                    // 剣もトゥーンで描画する
    m_swordObj->SetOutline(true, VECTOR4(0.0f, 0.0f, 0.0f, 1.0f), 1.5f);

    animator->SetModel(mesh);
    animator->Play(aIdle);

    // トゥーンシェーディングと輪郭線を有効にする
    SetToonEnabled(true);
    SetOutline(true, VECTOR4(0.0f, 0.0f, 0.0f, 1.0f), 2.0f);

    meshCol = new MeshCollider();
    meshCol->MakeFromMesh(mesh, animator);

    m_speed = 0.0f;
    m_velocity = VECTOR3(0, 0, 0);

    m_maxHitPoint = 1000;
    m_hitPoint = m_maxHitPoint;

    Serializer serializer;

    if (!serializer.Load("default_graph.json", m_graphData)) {
        std::cerr << "ERROR: Failed to load behavior tree graph from "
            "default_graph.json\n";
    }
    else {
        if (!m_btRunner.LoadGraph(m_graphData)) {
            std::cerr << "ERROR: Executor failed to load behavior tree graph."
                << std::endl;
        }
    }

    m_btRunner.SetExeContext(this);

    m_btRunner.ResetTree();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 破棄
Executor::~Executor() {
  SAFE_DELETE(mesh);
  SAFE_DELETE(meshCol);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 更新
void Executor::Update() {
  if (m_state == stDead) {
    animator->Update();
    return;
  }

  ValidateTargets();

  m_prevstate = m_state;

  // StAttack中もBTを継続実行（ExecuteAttackActionがRUNNINGを返す間は他ノードに進まない）
  m_btRunner.Execute();

  UpdateState();
  UpdateAnimation();
  ApplyGravityAndMap();

  animator->Update();

  if (m_swordObj) {
    m_swordObj->Update();
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 描画
void Executor::Draw() { Object3D::Draw(); }
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 直線とメッシュの当たり判定
bool Executor::Raycast(const VECTOR3 &start, const VECTOR3 &end,
                       MeshCollider::CollInfo *info) {
  std::list<MapBase *> maps = ObjectManager::FindGameObjects<MapBase>();
  bool hit = false;
  float minDistance = FLT_MAX;
  MeshCollider::CollInfo tempInfo;

  for (MapBase *map : maps) {
    if (map->HitLineToMesh(start, end, &tempInfo)) {
      float dist = (tempInfo.hitPosition - start).Length();
      if (dist < minDistance) {
        minDistance = dist;
        hit = true;
        if (info) {
          *info = tempInfo;
        }
      }
    }
  }
  return hit;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 状態更新
void Executor::UpdateState() {
  transform.position += m_velocity;

  if (m_velocity.Length() > 0.0001f) {
    // std::cout << "DEBUG: Velocity(" << m_velocity.x << ", " << m_velocity.y
    // << ", " << m_velocity.z << ")" << std::endl;
  }

  if (m_state == State::StAttack) {
    UpdateAttack();
  }

  if (m_state != m_prevstate) {
    char buf[256];
    sprintf_s(buf, "DEBUG: State Change %d -> %d\n", m_prevstate, m_state);
    std::cout << buf;
  }

  if (m_hitPoint <= 0 && m_state != stDead) {
    m_state = stDead;
  }

  // DEBUG: Track EnemyMecha_Human Position
  static int dbgFrame = 0;
  dbgFrame++;
  if (dbgFrame % 60 == 0) { // Every ~1 sec
    auto enemies = ObjectManager::FindGameObjects<Object3D>();
    for (auto *obj : enemies) {
      if (obj->IsTag("Enemy")) {
        VECTOR3 pos = obj->Position();
        char msg[256];
        sprintf_s(msg, "DEBUG MONITOR: Enemy(%p) Pos(%.2f, %.2f, %.2f)\n", obj,
                  pos.x, pos.y, pos.z);
        OutputDebugStringA(msg);
      }
    }
    VECTOR3 myPos = transform.position;
    char myMsg[256];
    sprintf_s(myMsg, "DEBUG MONITOR: Executor Pos(%.2f, %.2f, %.2f)\n", myPos.x,
              myPos.y, myPos.z);
    OutputDebugStringA(myMsg);
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 重力とマップ衝突の適用
void Executor::ApplyGravityAndMap() {
  VECTOR3 posold = transform.position;

  transform.position.y += m_speed;
  m_speed -= gravity * 60 * SceneManager::DeltaTime();

  if (auto *mm = ObjectManager::FindGameObject<MapManager>()) {
    VECTOR3 beforeMap = transform.position;
    if (mm->IsCollisionMoveGravity(posold, transform.position) != clFall) {
      m_speed = 0.0f;
    }
    float moveDist = (transform.position - beforeMap).Length();
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// アニメーション更新
void Executor::UpdateAnimation() {
  if (m_state != m_prevstate) {
    switch (m_state) {
    case stIdle:
      animator->Play(aIdle);
      break;
    case stRun:
      animator->Play(aRun);
      break;
    case StAttack:
      animator->Play(aAttack1);
      break;
    case stDamage:
      animator->Play(aIdle);
      break;
    case stDead:
      animator->Play(aDead);
      break;
    }
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 攻撃更新
void Executor::UpdateAttack() {
  // Friction for lunge execution
  if (m_velocity.Length() > 0.001f) {
    m_velocity *= 0.80f; // Stronger friction for precise stop
  } else {
    m_velocity = VECTOR3(0, 0, 0);
  }

  switch (m_attackPhase) {
  case AttackPhase::phWait:
    if (animator->CurrentFrame() >= ATTACK_HIT_FRAME) {
      m_attackPhase = AttackPhase::phAttack;
      m_swordObj->EnableCollision(true);
    }
    break;

  case AttackPhase::phAttack:
    // Damage is handled by m_swordObj->Update() -> HitCheckLine
    m_attackPhase = AttackPhase::phEnd;
    break;

  case AttackPhase::phEnd:
    if (animator->Finished()) {
      m_swordObj->SetActive(false);
      m_swordObj->EnableCollision(false);
      m_state = State::stIdle;
      m_attackPhase = AttackPhase::phWait;
      std::cout << "INFO: Attack finished." << std::endl;
    }
    break;
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 前方移動
void Executor::MoveForward(float speed) {
  if (m_state == State::StAttack || m_state == State::stDead) {
    return;
  }

  VECTOR3 forward(0, 0, moveSpeedBase * speed * 60 * SceneManager::DeltaTime());
  MATRIX4X4 rotY = XMMatrixRotationY(transform.rotation.y);
  m_velocity = forward * rotY;

  if (m_state != State::stRun) {
    m_state = State::stRun;
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ナビメッシュを使用した移動
void Executor::MoveTo(const VECTOR3 &targetPos, float speed) {
   float d = (targetPos - m_currentNavTarget).Length();

  if (d > 1.0f || m_currentPath.empty()) {
    m_currentNavTarget = targetPos;

    NavigationManager::GetInstance().FindPath(transform.position, targetPos,
                                              m_currentPath);
    m_currentPathIndex = 0;

    if (!m_currentPath.empty()) {
      // Skip first point if it is too close (start point)
      if ((m_currentPath[0] - transform.position).Length() < 0.5f &&
          m_currentPath.size() > 1) {
        m_currentPathIndex = 1;
      }
    }
  }

  if (m_currentPath.empty()) {
    VECTOR3 myPos = transform.position;
    float dx = targetPos.x - myPos.x;
    float dz = targetPos.z - myPos.z;
    float targetAngle = atan2f(dx, dz);
    transform.rotation.y = targetAngle;
    MoveForward(speed);
    return;
  }

  FollowPath(speed);
}
//-----------------------------------------------------------------------------

// --------------------------------------------------------------------------
// ナビメッシュを使用した移動（ターゲットに向かって直進、パスは無視）
void Executor::MoveStraightTo(const VECTOR3 &targetPos, float speed) {
    // ナビメッシュを使用した経路探索
    float d = (targetPos - m_currentNavTarget).Length();

    // ターゲットが変わった、またはパスがない場合に再探索
    if (d > 1.0f || m_currentPath.empty()) {
        m_currentNavTarget = targetPos;
        NavigationManager::GetInstance().FindPath(transform.position, targetPos,
            m_currentPath);
        m_currentPathIndex = 0;

        if (!m_currentPath.empty()) {
            // 始点が近すぎる場合はスキップ
            if ((m_currentPath[0] - transform.position).Length() < 0.5f &&
                m_currentPath.size() > 1) {
                m_currentPathIndex = 1;
            }
        }
    }

    if (!m_currentPath.empty()) {
        FollowPath(speed);
        return;
    }

    VECTOR3 myPos = transform.position;
    float dx = targetPos.x - myPos.x;
    float dz = targetPos.z - myPos.z;

    // ターゲットが近すぎる場合は停止
    if (dx * dx + dz * dz < 0.01f) {
        StopMove();
        return;
    }

    // ターゲットに向かって直進
    float targetAngle = atan2f(dx, dz);

    // スムーズな回転（単純な線形補間、またはスナップ）
    float currentAngle = transform.rotation.y;
    float diff = targetAngle - currentAngle;
    while (diff > XM_PI)
        diff -= XM_2PI;
    while (diff < -XM_PI)
        diff += XM_2PI;

    // 直進はするが、回転はゆっくり目にする
    transform.rotation.y += diff * 0.2f;

    MoveForward(speed);
}
// --------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ナビメッシュに沿って移動
void Executor::FollowPath(float speed) {
    // パスがない、または最後まで到達している場合は停止
    if (m_currentPath.empty() || m_currentPathIndex >= m_currentPath.size()) {
        std::cout << "DEBUG: FollowPath Finished/Empty" << std::endl;
        StopMove();
        return;
    }

    // 現在のターゲットポイントに向かって移動
    VECTOR3 targetPoint = m_currentPath[m_currentPathIndex];
    VECTOR3 myPos = transform.position;

    // ターゲットポイントに近づいたら次のポイントへ
    float dx = targetPoint.x - myPos.x;
    float dz = targetPoint.z - myPos.z;
    float dist = sqrt(dx * dx + dz * dz);

    // ターゲットポイントに近すぎる場合は次のポイントへ
    if (dist < 0.5f) {
        m_currentPathIndex++;
        if (m_currentPathIndex >= m_currentPath.size()) {
            StopMove();
            return;
        }

        // 次のターゲットポイントに向かう
        targetPoint = m_currentPath[m_currentPathIndex];
        dx = targetPoint.x - myPos.x;
        dz = targetPoint.z - myPos.z;
    }

    // ターゲットポイントに向かって回転
    float targetAngle = atan2f(dx, dz);

    // 回転
    float currentAngle = transform.rotation.y;
    float diff = targetAngle - currentAngle;

    // 角度の差を-π～πの範囲に収める
    while (diff > XM_PI)
        diff -= XM_2PI;
    while (diff < -XM_PI)
        diff += XM_2PI;

    // 回転速度を調整（単純な線形補間、またはスナップ）
    transform.rotation.y += diff * 0.1f; // Turn speed

    // 前方移動をリクエスト
    MoveForward(speed);
}

//-----------------------------------------------------------------------------
// 移動停止
void Executor::StopMove()
{
    m_velocity = VECTOR3{ 0, 0, 0 };

    if (m_state == State::stRun) {
        m_state = State::stIdle;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 方向転換
void Executor::TurnTo(const VECTOR3 &dir) 
{
    VECTOR3 d = dir;
    d.y = 0;
    if (d.Length() > 0.0001f) {
        d = XMVector3Normalize(d);
        float angle = atan2f(d.x, d.z);
        transform.rotation.y = angle;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 次の攻撃スキルをセット
void Executor::SetNextAttackSkill(int skillId, float range, float damage,
                                  int score) 
{
    m_nextSkillId = skillId;
    m_nextAttackRange = range;
    m_nextAttackDamage = damage;
    m_nextSkillScore = score;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 攻撃開始
void Executor::StartAttack(TargetBoxType target) {
	// すでに攻撃中または死亡している場合は無視
    if (m_state == State::StAttack || m_state == State::stDead) {
        return;
    }

	// 攻撃開始のセットアップ
    m_swordObj->SetActive(true);
    m_swordObj->EnableCollision(false);
    m_attackRange = m_nextAttackRange;
    m_attackDamage = m_nextAttackDamage;
    m_attackTargetBox = target;
    m_currentSkillId = m_nextSkillId;
    m_currentUseSkillScore = m_nextSkillScore;
    m_attackTimer = 0.0f;
    m_hasDealtDamage = false;
    m_state = State::StAttack;
    m_velocity = VECTOR3(0, 0, 0);

	// 攻撃対象がいる場合は、そちらを向く
    if (HasTargetsInBox(target)) {
        VECTOR3 targetPos = GetTargetPositionFromBox(target);
        VECTOR3 dir = targetPos - transform.position;
        TurnTo(targetPos - transform.position);

		// Slashの場合は、攻撃開始と同時に突進も行う
        if (m_currentSkillId == 2) {
            VECTOR3 diff = targetPos - transform.position;
            diff.y = 0;

			// ターゲットとの水平距離がある程度以上ある場合のみ突進を行う
            if (diff.Length() > 0.1f) {
				// ターゲットとの水平距離を計算
                float dist = diff.Length();
				// ターゲットとの距離が1.0f以上ある場合は、1.0f分を引いた距離を基準に突進速度を計算
                float targetDist = (dist > 1.0f) ? (dist - 1.0f) : 0.0f;

				// 距離に応じた突進速度を計算（最大0.6f）
                float lungeSpeed = targetDist * 0.15f;
                if (lungeSpeed > 0.6f)
                    lungeSpeed = 0.6f;

				// ターゲットに向かって突進速度を設定
                m_velocity = XMVector3Normalize(diff) * lungeSpeed;

				// ジャンプ力も少し加える（距離に応じて最大0.15f）
                m_speed = 0.15f; // Reduced Jump Force
            }
        }
    }

	// アニメーション再生
    if (m_currentSkillId == 1)
    {
        animator->Play(aAttack2);
    }
    else if (m_currentSkillId == 2)
    {
        animator->Play(aAttack3);
    }
    else // Default
    {
        animator->Play(aAttack1);
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 地面にいるか
bool Executor::IsOnGround() const { return (m_speed == 0.0f); }
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ダメージを受ける
void Executor::AddDamage(float damage, const VECTOR3 &fromPos) {
	// 死亡状態なら無視
    if (m_state == stDead)
        return;

	// ダメージを減算
    m_hitPoint -= (int)damage;
    if (m_hitPoint <= 0) {
        m_hitPoint = 0;
        m_state = stDead;
        animator->Play(2);
    }

	// ダメージを受けた方向に少し押される
    else {
        VECTOR3 push = transform.position - fromPos;
        push.y = 0;
		// ターゲットとの水平距離がある程度以上ある場合のみ押し戻しを行う
        if (push.Length() > 0.0001f) {
            push = XMVector3Normalize(push) * 0.2f;
            transform.position += push;
        }
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ターゲットをボックスに追加
void Executor::AddToTargetBox(TargetBoxType boxType, Object3D *target) {
	// 無効なボックスタイプやターゲットがnullptrの場合は無視
    if (target == nullptr)
        return;
    if (target == this)
        return;
    if (boxType == TargetBoxType::None || boxType == TargetBoxType::Position)
        return;

    // 重複チェック
    if (m_targetBoxSets[boxType].find(target) == m_targetBoxSets[boxType].end()) {
        m_targetBoxes[boxType].push_back(target);
        m_targetBoxSets[boxType].insert(target);

        // Allボックスにも追加（他のボックスに追加された場合）
        if (boxType != TargetBoxType::All) {
            if (m_targetBoxSets[TargetBoxType::All].find(target) ==
                m_targetBoxSets[TargetBoxType::All].end()) {
                m_targetBoxes[TargetBoxType::All].push_back(target);
                m_targetBoxSets[TargetBoxType::All].insert(target);
            }
        }
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ターゲットボックスをクリア
void Executor::ClearTargetBox(TargetBoxType boxType) {
	// NoneやPositionの場合は特別な処理
    if (boxType == TargetBoxType::Position) {
        ClearManualTargetPosition();
        return;
    }

	// 指定されたボックスタイプのターゲットをクリア
    m_targetBoxes[boxType].clear();
    m_targetBoxSets[boxType].clear();
}

//-----------------------------------------------------------------------------
// 全てのターゲットボックスをクリア
void Executor::ClearAllTargetBoxes() {
    m_targetBoxes.clear();
    m_targetBoxSets.clear();
    ClearManualTargetPosition();
    m_currentRadarRadius = 0.0f;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ターゲットボックスから最も近いターゲットを取得
Object3D *Executor::GetNearestFromBox(TargetBoxType boxType) const {
	// NoneやPositionの場合はターゲットが存在しないためnullptrを返す
    if (boxType == TargetBoxType::None || boxType == TargetBoxType::Position) {
        return nullptr;
    }

	// 指定されたボックスタイプのターゲットリストを取得
    auto it = m_targetBoxes.find(boxType);
    if (it == m_targetBoxes.end() || it->second.empty()) {
        OutputDebugStringA("DEBUG: GetNearestFromBox - List empty or not found.\n");
        return nullptr;
    }

	// DEBUG: ターゲットリストの内容を出力
    char listDbg[512];
    sprintf_s(listDbg, "DEBUG: GetNearestFromBox List [Type:%d, Size:%zu]:\n",
        (int)boxType, it->second.size());
    OutputDebugStringA(listDbg);

	// 最も近いターゲットを見つけるための変数
    Object3D* nearest = nullptr;
    float minDistSq = FLT_MAX;

	// ターゲットリストを走査
    for (Object3D* target : it->second) {
		// ターゲットがnullptrの場合はスキップ（ValidateTargetsで削除されている可能性がある）
        if (target == nullptr) {
            OutputDebugStringA("  - NULL Target\n");
            continue;
        }

		// ターゲットのクラス名と位置をデバッグ出力
        VECTOR3 tPos = target->Position();
        const char* clsName = typeid(*target).name();

		// ターゲットのクラス名を簡略化（例: "class EnemyMecha_Human" -> "EnemyMecha_Human"）
        char itemDbg[256];
        sprintf_s(itemDbg, "  - Class[%s] Pos(%.2f, %.2f, %.2f)\n", clsName, tPos.x,
            tPos.y, tPos.z);
        OutputDebugStringA(itemDbg);

		// ターゲットとの距離の二乗を計算（平方根を取らずに比較できるようにする）
        VECTOR3 diff = tPos - transform.position;
        float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		// 最も近いターゲットを更新
        if (distSq < minDistSq) {
            minDistSq = distSq;
            nearest = target;
        }
    }

    return nearest;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ターゲットボックスからターゲットの座標を取得
VECTOR3 Executor::GetTargetPositionFromBox(TargetBoxType boxType) const {
  // Positionボックスの場合は手動設定座標を返す
  if (boxType == TargetBoxType::Position) {
    return m_manualTargetPosition;
  }

  // 他のボックスの場合は最も近いターゲットの座標を返す
  Object3D *nearest = GetNearestFromBox(boxType);
  if (nearest) {
    return nearest->Position();
  }

  return VECTOR3(0.0f, 0.0f, 0.0f);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ターゲットの有効性を検証し、死んでいる（削除された）ターゲットをボックスから削除
void Executor::ValidateTargets() {
	// 現在アクティブなObject3Dのセットを構築
    std::list<Object3D*> activeObjs = ObjectManager::FindGameObjects<Object3D>();
    std::set<Object3D*> activeSet;

	// アクティブなオブジェクトをセットに追加
    for (auto* obj : activeObjs) {
        if (obj)
            activeSet.insert(obj);
    }

	// ターゲットボックスを走査し、アクティブでない（削除された）ターゲットを削除
    for (auto& pair : m_targetBoxes) {
        auto& vec = pair.second;
        auto it = std::remove_if(vec.begin(), vec.end(), [&](Object3D* target) {
			// ターゲットがnullptrの場合は削除対象
            if (target == nullptr) return true;

			// ターゲットがアクティブなオブジェクトセットに存在しない場合は削除対象
            if (activeSet.find(target) == activeSet.end()) {
                m_targetBoxSets[pair.first].erase(target);
                return true;
            }
            return false;
            });

		// remove_ifで削除対象を末尾に移動させた後、実際にベクターから削除
        if (it != vec.end()) {
            vec.erase(it, vec.end());
        }
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ターゲットボックスに有効なターゲットが存在するか
bool Executor::HasTargetsInBox(TargetBoxType boxType) const {
	// Noneボックスは常にターゲットなし、Positionボックスは手動座標の有無で判定
    if (boxType == TargetBoxType::None) {
        return false;
    }

	// Positionボックスは手動で座標が設定されているかどうかで判定
    if (boxType == TargetBoxType::Position) {
        return m_hasManualTargetPosition;
    }

	// 他のボックスはターゲットリストに有効なターゲットが存在するかどうかで判定
    auto it = m_targetBoxes.find(boxType);
    return (it != m_targetBoxes.end() && !it->second.empty());
}

//-----------------------------------------------------------------------------
// ターゲットボックス内のターゲット数を取得
int Executor::GetTargetCountInBox(TargetBoxType boxType) const {
	// Noneボックスは常に0、Positionボックスは手動座標の有無で1または0を返す
    if (boxType == TargetBoxType::None) {
        return 0;
    }

	// Positionボックスは手動で座標が設定されているかどうかで1または0を返す
    if (boxType == TargetBoxType::Position) {
        return m_hasManualTargetPosition ? 1 : 0;
    }

	// 他のボックスはターゲットリストのサイズを返す（存在しない場合は0）
    auto it = m_targetBoxes.find(boxType);
    if (it != m_targetBoxes.end()) {
        return static_cast<int>(it->second.size());
    }

    return 0;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ターゲットボックス内の全てのターゲットを取得
const std::vector<Object3D *> &
Executor::GetTargetBox(TargetBoxType boxType) const {
  if (boxType == TargetBoxType::None || boxType == TargetBoxType::Position) {
    return s_emptyTargetList;
  }

  auto it = m_targetBoxes.find(boxType);
  if (it != m_targetBoxes.end()) {
    return it->second;
  }
  return s_emptyTargetList;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 手動でターゲット位置を設定
void Executor::SetManualTargetPosition(const VECTOR3 &pos) {
  m_manualTargetPosition = pos;
  m_hasManualTargetPosition = true;
  std::cout << "INFO: Manual target position set to (" << pos.x << ", " << pos.y
            << ", " << pos.z << ")" << std::endl;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 手動で設定されたターゲット位置をクリア
void Executor::ClearManualTargetPosition() {
  m_manualTargetPosition = VECTOR3(0.0f, 0.0f, 0.0f);
  m_hasManualTargetPosition = false;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ビヘイビアツリー関連の関数
std::string Executor::GetInputSourceNodeId(const std::string &nodeId,
                                           const std::string &pinName) const {
    return m_btRunner.GetInputSourceNodeId(nodeId, pinName);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ノードインスタンスを取得
const NodeInstance *Executor::GetNodeInstance(const std::string &nodeId) const {
    return m_btRunner.GetNodeInstance(nodeId);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 前方ベクトルを取得
VECTOR3 Executor::GetForwardVector() const {
    // ワールド行列のZ軸成分（前方ベクトル）を取得
    MATRIX4X4 world = transform.matrix();
    VECTOR3 forward(world._31, world._32, world._33);
    return normalize(forward);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ターゲットが視野内にいるかを判定
bool Executor::IsTargetInFOV(const Object3D *target,
                             float fovAngleDegrees) const {
	// ターゲットがnullptrの場合は視野内とみなさない
    if (!target)
        return false;

    VECTOR3 toTarget = target->Position() - this->Position();

    // Y軸（高さ）の影響を無視して水平方向の視野のみを考慮する場合
    toTarget.y = 0.0f;

    float distSq = magnitudeSQ(toTarget);
    if (distSq < 0.0001f) {
        // ほぼ同じ位置にいる場合は視野内とみなす（あるいは判定不能）
        return true;
    }

    toTarget = normalize(toTarget);
    VECTOR3 forward = GetForwardVector();
    forward.y = 0.0f; // 前方ベクトルのY成分も無視（水平視野）
    forward = normalize(forward);

    float dotProd = Dot(forward, toTarget);

    // 視野角の半分（中心から左右への角度）のコサイン値を計算
    // 角度が広いほどコサイン値は小さくなる（例：90度視野なら左右45度 =>
    // cos(45)）
    float halfAngleRad = (fovAngleDegrees * 0.5f) * DegToRad;
    float threshold = cosf(halfAngleRad);

    // 内積が閾値より大きければ、角度は閾値より小さい（＝視野内）
    return dotProd >= threshold;
}
//-----------------------------------------------------------------------------