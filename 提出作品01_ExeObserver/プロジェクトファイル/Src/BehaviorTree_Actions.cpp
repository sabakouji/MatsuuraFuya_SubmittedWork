#include "BehaviorTree_Actions.h"
#include "BehaviorTree_Nodes.h"
#include "Executor.h"
#include "Goal.h"
#include "MeshCollider.h"
#include "NodeEvaluator.h"
#include "ObjectManager.h"
#include <iostream>

#include "EnemyBase.h"
#include "EnemyManager.h"
#include "EventBase.h"
#include "Item.h"
#include "Object3D.h"
#include "SceneManager.h"
#include <cmath>
#include <iostream>

//-----------------------------------------------------------------------------
// RunActionの実行
BehaviorState RunAction::ExecuteAction(Executor* executorContext) {
    // 安全チェック
    if (executorContext == nullptr) {
        std::cerr << "ERROR: RunAction failed: Executor context is null."
            << std::endl;
        return BehaviorState::FAILURE;
    }

	// デバッグ：現在実行中のノード名をExecutorに伝える
    char startDbg[256];
    sprintf_s(startDbg, "DEBUG: RunAction Start. Box:%d Dist:%.2f Speed:%.2f\n",
        (int)m_targetBox, m_arrivalDistance, m_speed);
    OutputDebugStringA(startDbg);

    // ボックスを使わない場合は前方移動
    if (m_targetBox == TargetBoxType::None) {
        OutputDebugStringA(
            "DEBUG: RunAction - TargetBox is None. Moving Forward blindly.\n");
        executorContext->MoveForward(m_speed);
        return BehaviorState::SUCCESS;
    }

    // 指定したボックスにターゲットが存在するか確認
    if (!executorContext->HasTargetsInBox(m_targetBox)) {
        // ボックスが空の場合は停止してFAILURE
        executorContext->StopMove();
        OutputDebugStringA(
            "DEBUG: RunAction - No targets in box, returning FAILURE\n");
        // std::cout << "DEBUG: RunAction - No targets in box, returning FAILURE" <<
        // std::endl;
        return BehaviorState::FAILURE;
    }

    // ボックスから目標座標を取得
    VECTOR3 targetPos = executorContext->GetTargetPositionFromBox(m_targetBox);
    VECTOR3 myPos = executorContext->Position();

	// デバッグ：座標を出力
    char coordDbg[256];
    sprintf_s(coordDbg,
        "DEBUG: RunAction Coords - MyPos(%.2f, %.2f, %.2f) Target(%.2f, "
        "%.2f, %.2f)\n",
        myPos.x, myPos.y, myPos.z, targetPos.x, targetPos.y, targetPos.z);
    OutputDebugStringA(coordDbg);

    // 水平方向の距離を計算（Y軸は無視）
    float dx = targetPos.x - myPos.x;
    float dz = targetPos.z - myPos.z;
    float distance = std::sqrt(dx * dx + dz * dz);

    // 到達判定
    if (distance <= m_arrivalDistance) {
        executorContext->StopMove();
        char arrDbg[256];
        sprintf_s(
            arrDbg,
            "INFO: RunAction - Arrived at target! Dist:%.2f <= Arrival:%.2f\n",
            distance, m_arrivalDistance);
        OutputDebugStringA(arrDbg);
        // std::cout << "INFO: RunAction - Arrived at target from box!" <<
        // std::endl;
        return BehaviorState::SUCCESS;
    }

    // ターゲットの方向を向いて移動
    // Use Pathfinding MoveTo (NavigationManager logic inside Executor)
    char dbg[256];
    sprintf_s(dbg, "DEBUG: RunAction Moving to (%.2f, %.2f) Dist: %.2f\n",
        targetPos.x, targetPos.z, distance);
    OutputDebugStringA(dbg);

	// MoveToはナビメッシュに沿って移動するが、RunActionはターゲットに向かって直進するため、MoveStraightToを使用
    executorContext->MoveStraightTo(targetPos, m_speed);

    return BehaviorState::RUNNING;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// StopActionの実行
BehaviorState StopAction::ExecuteAction(Executor* executorContext) {
	// 安全チェック
    if (executorContext == nullptr) {
        std::cerr << "ERROR: Action failed: Executor context is null." << std::endl;
        return BehaviorState::FAILURE;
    }

    // 移動を停止
    executorContext->StopMove();

    return BehaviorState::SUCCESS;
}
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// RadarScanActionの実行
void RadarScanAction::Initialize() {
    ActionNode::Initialize();
    m_currentRadius = 0.0f;
    m_elapsedTime = 0.0f;
    m_isScanning = false;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// オブジェクトを分類してターゲットボックスの種類を返す
TargetBoxType RadarScanAction::ClassifyObject(Object3D* obj) const {
	// 安全チェック
    if (!obj)
        return TargetBoxType::None;

	// タグ判定（EnemyBase::EnemyBase()でSetTag("Enemy")済み）
    if (obj->IsTag("Enemy")) {
        return TargetBoxType::Enemy;
    }

    // ゴール判定
    if (dynamic_cast<Goal*>(obj) != nullptr) {
        return TargetBoxType::Goal;
    }

    // ドア判定
    if (dynamic_cast<EventBase*>(obj) != nullptr) {
        return TargetBoxType::Door;
    }

    // アイテム判定
    if (dynamic_cast<Item*>(obj) != nullptr) {
        return TargetBoxType::Item;
    }

    return TargetBoxType::All;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// レーダースキャンの実行
BehaviorState RadarScanAction::ExecuteAction(Executor* executorContext) {
	// 安全チェック
    if (executorContext == nullptr) {
        return BehaviorState::FAILURE;
    }

	// スキャン開始
    float currentRadius = executorContext->GetCurrentRadarRadius();
    float deltaTime = SceneManager::DeltaTime();

	// スキャン開始前の初期化
    if (currentRadius <= 0.1f) {
        currentRadius = 1.0f;
    }

	// 半径を拡大
    currentRadius +=
        (currentRadius * m_expandSpeed * deltaTime) + (2.0f * deltaTime);

	// スキャン範囲内のオブジェクトを検出して分類
    if (currentRadius >= m_maxRadius) {
        currentRadius = m_maxRadius;
    }

	// スキャン処理
    m_currentRadius = currentRadius;
    executorContext->SetCurrentRadarRadius(currentRadius);

    return BehaviorState::SUCCESS;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// スキャン範囲内のオブジェクトを検出して分類
void RadarScanAction::ScanObjectsInRadius(Executor* executorContext) {
	// 安全チェック
    if (!executorContext)
        return;

	// 前回のスキャン結果をクリア
    executorContext->ClearTargetBox(TargetBoxType::Enemy);
    executorContext->ClearTargetBox(TargetBoxType::Item);
    executorContext->ClearTargetBox(TargetBoxType::Door);
    executorContext->ClearTargetBox(TargetBoxType::Goal);

    // スキャン範囲内のオブジェクトを検出して分類
    VECTOR3 centerPos = executorContext->Position();
    float radiusSq = m_currentRadius * m_currentRadius;

	// 全オブジェクトを取得（最適化の余地あり：Spatial Partitioningなど）
    std::list<Object3D*> allObjects = ObjectManager::FindGameObjects<Object3D>();

    // スキャン範囲内のオブジェクトを検出
    for (Object3D* obj : allObjects) {
		// 自分自身はスキャン対象外
        if (obj == static_cast<Object3D*>(executorContext))
            continue;

		// 水平方向の距離を計算（Y軸は無視）
        VECTOR3 objPos = obj->Position();
        float dx = objPos.x - centerPos.x;
        float dz = objPos.z - centerPos.z;
        float distSq = dx * dx + dz * dz;

		// 半径内にあるか判定
        if (distSq <= radiusSq) {
            // オブジェクトを分類
            TargetBoxType objType = ClassifyObject(obj);
            executorContext->AddToTargetBox(objType, obj);
        }
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// パルス式レーダーの実行
void RadarPulseAction::Initialize() {
    ActionNode::Initialize();
    m_hasScanned = false;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// パルス式レーダーのオブジェクト分類
TargetBoxType RadarPulseAction::ClassifyObject(Object3D* obj) const {
	// 安全チェック
    if (!obj)
        return TargetBoxType::None;

	// タグ判定（EnemyBase::EnemyBase()でSetTag("Enemy")済み；EnemyManagerは非Enemy）
    if (obj->IsTag("Enemy")) {
        return TargetBoxType::Enemy;
    }

	// ゴール判定
    if (dynamic_cast<Goal*>(obj) != nullptr) {
        return TargetBoxType::Goal;
    }

	// ドア判定
    if (dynamic_cast<EventBase*>(obj) != nullptr) {
        return TargetBoxType::Door;
    }

	// アイテム判定
    if (dynamic_cast<Item*>(obj) != nullptr) {
        return TargetBoxType::Item;
    }

    return TargetBoxType::All;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// パルス式レーダーの実行
BehaviorState RadarPulseAction::ExecuteAction(Executor* executorContext) {
	// 安全チェック
    if (executorContext == nullptr) {
        return BehaviorState::FAILURE;
    }

    // スキャン処理
    executorContext->ClearAllTargetBoxes();
    executorContext->SetCurrentRadarRadius(m_radius);

	// スキャン範囲内のオブジェクトを検出して分類
    VECTOR3 centerPos = executorContext->Position();
    float radiusSq = m_radius * m_radius;

	// 全オブジェクトを取得（最適化の余地あり：Spatial Partitioningなど）
    std::list<Object3D*> allObjects = ObjectManager::FindGameObjects<Object3D>();

	// デバッグ用カウンタ
    int foundCount = 0;
    int enemyCount = 0;
    int doorCount = 0;
    int itemCount = 0;

    // スキャン範囲内のオブジェクトを検出
    for (Object3D* obj : allObjects) {
		// 自分自身はスキャン対象外
        if (obj == static_cast<Object3D*>(executorContext))
            continue;

        VECTOR3 objPos = obj->Position();

        float dx = objPos.x - centerPos.x;
        float dz = objPos.z - centerPos.z;
        float distSq = dx * dx + dz * dz;

		// 半径内にあるか判定
        if (distSq <= radiusSq) {
            TargetBoxType objType = ClassifyObject(obj);
            executorContext->AddToTargetBox(objType, obj);
            foundCount++;

            switch (objType) {
            case TargetBoxType::Enemy:
                enemyCount++;
                break;
            case TargetBoxType::Door:
                doorCount++;
                break;
            case TargetBoxType::Item:
                itemCount++;
                break;
            default:
                break;
            }
        }
    }

	// デバッグ：スキャン結果を出力
    char dbg[256];
    sprintf_s(dbg,
        "INFO: RadarPulse found %d targets. (Enemy:%d, Door:%d, Item:%d, "
        "Goal:%d)\n",
        foundCount, enemyCount, doorCount, itemCount,
        0);

    // スキャン完了
    bool hasTargets = executorContext->HasTargetsInBox(TargetBoxType::All);
    return hasTargets ? BehaviorState::SUCCESS : BehaviorState::FAILURE;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SetTargetPositionActionの実行
BehaviorState
SetTargetPositionAction::ExecuteAction(Executor* executorContext) {
	// 安全チェック
    if (executorContext == nullptr) {
        return BehaviorState::FAILURE;
    }

	// ターゲット座標を設定
    VECTOR3 targetPos(m_x, m_y, m_z);
    executorContext->SetManualTargetPosition(targetPos);

    std::cout << "INFO: SetTargetPosition - Target set to (" << m_x << ", " << m_y
        << ", " << m_z << ")" << std::endl;

    return BehaviorState::SUCCESS;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ClearTargetBoxActionの実行
BehaviorState ClearTargetBoxAction::ExecuteAction(Executor* executorContext) {
	// 安全チェック
    if (executorContext == nullptr) {
        return BehaviorState::FAILURE;
    }

	// ターゲットボックスをクリア
    if (m_boxType == TargetBoxType::All || m_boxType == TargetBoxType::None) {
        executorContext->ClearAllTargetBoxes();
        std::cout << "INFO: ClearTargetBox - All boxes cleared" << std::endl;
    }
	// 指定されたボックスのみクリア
    else {
        executorContext->ClearTargetBox(m_boxType);
        std::cout << "INFO: ClearTargetBox - Box cleared" << std::endl;
    }

    return BehaviorState::SUCCESS;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HasTargetsInBoxConditionの実行
BehaviorState
HasTargetsInBoxCondition::ExecuteAction(Executor* executorContext) {
	// 安全チェック
    if (executorContext == nullptr) {
        return BehaviorState::FAILURE;
    }

	// 指定されたボックス内のターゲット数を取得
    int count = executorContext->GetTargetCountInBox(m_boxType);
    return (count >= m_minTargets) ? BehaviorState::SUCCESS
        : BehaviorState::FAILURE;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// IsNearTargetConditionの実行
BehaviorState IsNearTargetCondition::ExecuteAction(Executor* executorContext) {
	// 安全チェック
    if (executorContext == nullptr) {
        return BehaviorState::FAILURE;
    }

	// 指定されたボックス内にターゲットが存在するか確認
    if (!executorContext->HasTargetsInBox(m_boxType)) {
        return BehaviorState::FAILURE;
    }

	// 自分の座標とターゲットの座標を取得
    VECTOR3 myPos = executorContext->Position();
    VECTOR3 targetPos = executorContext->GetTargetPositionFromBox(m_boxType);

    float dx = targetPos.x - myPos.x;
    float dz = targetPos.z - myPos.z;
    float distance = std::sqrt(dx * dx + dz * dz);

    return (distance <= m_distance) ? BehaviorState::SUCCESS
        : BehaviorState::FAILURE;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// InTheEyesConditionの実行
BehaviorState InTheEyesCondition::Execute(Executor* executorContext) {
    if (executorContext == nullptr) {
        return BehaviorState::FAILURE;
    }

    // 1. 指定されたボックス内のターゲットを取得
    const auto& targets = executorContext->GetTargetBox(m_boxType);
    if (targets.empty()) {
        // ターゲットがいない場合は false ルートへ
        if (m_falseNode)
            return m_falseNode->Execute(executorContext);
        return BehaviorState::FAILURE;
    }

    // 2. 各ターゲットについて視野判定
    bool anyVisible = false;
    for (const auto* target : targets) {
        if (executorContext->IsTargetInFOV(target, m_angle)) {
            anyVisible = true;
            break;
        }
    }

    if (anyVisible) {
        if (m_trueNode)
            return m_trueNode->Execute(executorContext);
        return BehaviorState::SUCCESS;
    }
    else {
        if (m_falseNode)
            return m_falseNode->Execute(executorContext);
        return BehaviorState::FAILURE;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CheckAttackRangeConditionの実行
IfNode::IfNode(const NodeInstance& id, const std::string& name)
    : ConditionNode(id, name) {
}

//-----------------------------------------------------------------------------
// Conditionピンに接続されたノードの評価結果に基づいて分岐するIfノードの実行
BehaviorState IfNode::Execute(Executor* executorContext) {
	// 安全チェック
    if (executorContext == nullptr) {
        return BehaviorState::FAILURE;
    }

	// デバッグログ
    bool conditionMet = false;

    // Conditionピンに接続されたノードIDを取得
    std::string sourceNodeId =
        executorContext->GetInputSourceNodeId(m_id.id, "Condition");

	// デバッグログ
    if (!sourceNodeId.empty()) {
        const NodeInstance* sourceNode =
            executorContext->GetNodeInstance(sourceNodeId);

        if (sourceNode) {
            conditionMet = NodeEvaluator::EvaluateBool(sourceNode, executorContext);
        }
    }

    // 分岐処理
    if (conditionMet) {
        if (m_trueNode)
            return m_trueNode->Execute(executorContext);
        return BehaviorState::SUCCESS;
    }
    else {
        if (m_falseNode)
            return m_falseNode->Execute(executorContext);
        return BehaviorState::FAILURE;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 攻撃範囲内にターゲットがいるか判定する条件ノードの実行
BehaviorState CheckAttackRangeCondition::Execute(Executor* executorContext) {
	// 安全チェック
    if (executorContext == nullptr) {
        return BehaviorState::FAILURE;
    }
    std::cout << "[DEBUG] CheckAttackRange Executing. Box: " << (int)m_targetBox
        << std::endl;

	// Conditionピンに接続されたノードから攻撃範囲を取得
    float attackRange = 1.0f; // Default range
    std::string skillNodeId =
        executorContext->GetInputSourceNodeId(m_id.id, "Skill");

    std::cout << "[DEBUG] CheckAttackRange: NodeId=" << m_id.id
        << " SkillNodeId=" << (skillNodeId.empty() ? "NONE" : skillNodeId)
        << std::endl;

	// デバッグログ
    if (skillNodeId.empty()) {
        std::cout << "[DEBUG] CheckAttackRange - No Skill Node connected. Using "
            "default range 1.0"
            << std::endl;
    }
    else {
        const NodeInstance* skillNode =
            executorContext->GetNodeInstance(skillNodeId);
        if (skillNode) {
            auto itRange = skillNode->properties.find("range");
            if (itRange != skillNode->properties.end()) {
                try {
                    attackRange = std::stof(itRange->second);
                    std::cout << "[DEBUG] CheckAttackRange - Found Skill Range: "
                        << attackRange << std::endl;
                }
                catch (...) {
                    attackRange = 1.0f; // Fallback on error
                    std::cout << "[DEBUG] CheckAttackRange - Skill Range Parse Error"
                        << std::endl;
                }
            }
            else {
                std::cout
                    << "[DEBUG] CheckAttackRange - Skill Node has no 'range' property"
                    << std::endl;
            }
        }
        else {
            std::cout << "[DEBUG] CheckAttackRange - Skill Node Instance Not Found"
                << std::endl;
        }
    }

    // ターゲットボックス内の敵を取得
    const auto& targets = executorContext->GetTargetBox(m_targetBox);
    std::cout << "[DEBUG] CheckAttackRange - TargetBox: " << (int)m_targetBox
        << " Count: " << targets.size() << std::endl;

    if (targets.empty()) {
        // 敵がいなければFalseルートへ
        std::cout
            << "[DEBUG] CheckAttackRange - No Targets -> Going to False Node (Run?)"
            << std::endl;
        if (m_falseNode) {
            std::cout << "[DEBUG] CheckAttackRange - Executing FalseNode"
                << std::endl;
            return m_falseNode->Execute(executorContext);
        }
        std::cout << "[DEBUG] CheckAttackRange - No FalseNode connected! Returning "
            "FAILURE"
            << std::endl;
        return BehaviorState::FAILURE;
    }

    VECTOR3 myPos = executorContext->Position();
    VECTOR3 eyePos = myPos;
    eyePos.y += 1.0f; // 自分の腰の高さ

    bool isInRange = false;

    std::cout << "[DEBUG] Target Count: " << targets.size() << std::endl;
    for (Object3D* obj : targets) {
        if (obj) {
            VECTOR3 p = obj->Position();
            std::cout << "[DEBUG] ObjPtr: " << obj
                << " Dist: " << (p - myPos).Length() << std::endl;
        }
        if (!obj)
            continue;

        // 敵との単純な距離を計算
        float simpleDist = (obj->Position() - myPos).Length();

        std::cout << "[DEBUG] CheckAttackRange - Target: " << obj
            << " Dist: " << simpleDist << " Range: " << attackRange
            << std::endl;

        // 単純距離チェック
        if (simpleDist > attackRange) {
            continue;
        }

        // もし「1.5m以内（密着状態）」なら、レイキャストを飛ばすまでもなく攻撃可能とみなす
        float closeRangeThreshold = 2.0f;

        if (simpleDist <= closeRangeThreshold) {
            isInRange = true;
            std::cout << "[DEBUG] CheckAttackRange - Target in Close Range!"
                << std::endl;
            break;
        }

        EnemyBase* enemy = dynamic_cast<EnemyBase*>(obj);
        if (enemy) {
            VECTOR3 targetPos = enemy->Position();
            targetPos.y += 1.0f;

            VECTOR3 toTarget = targetPos - eyePos;
            VECTOR3 dir = XMVector3Normalize(toTarget);

            // 判定距離マージン
            float safetyMargin = 0.9f;
            float checkDist = attackRange * safetyMargin;

            VECTOR3 rayEnd = eyePos + (dir * checkDist);
            MeshCollider::CollInfo info;

            if (enemy->HitLineToMesh(eyePos, rayEnd, &info)) {
                isInRange = true;
                std::cout << "[DEBUG] CheckAttackRange - Raycast Hit Enemy!"
                    << std::endl;
                break;
            }
            else {
                std::cout << "[DEBUG] CheckAttackRange - Raycast Missed" << std::endl;
            }
        }
        else {
            // 敵以外のオブジェクトへのフォールバック
            if (simpleDist <= attackRange * 0.9f) {
                isInRange = true;
                break;
            }
        }
    }

    if (isInRange) {
        // 射程内ならTrueルート（攻撃実行など）
        std::cout
            << "[DEBUG] CheckAttackRange - In Range -> Going to True Node (Attack?)"
            << std::endl;
        if (m_trueNode) {
            std::cout << "[DEBUG] CheckAttackRange - Executing TrueNode" << std::endl;
            return m_trueNode->Execute(executorContext);
        }
        return BehaviorState::SUCCESS;
    }
    else {
        // 射程外ならFalseルート（接近移動など）
        std::cout << "[DEBUG] CheckAttackRange - Out of Range -> Going to False "
            "Node (Run?)"
            << std::endl;
        if (m_falseNode) {
            std::cout << "[DEBUG] CheckAttackRange - Executing FalseNode"
                << std::endl;
            return m_falseNode->Execute(executorContext);
        }
        std::cout << "[DEBUG] CheckAttackRange - No FalseNode connected! Returning "
            "FAILURE"
            << std::endl;
        return BehaviorState::FAILURE;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 攻撃アクションの実行
void ExecuteAttackAction::Initialize() {
    ActionNode::Initialize();
    m_isAttacking = false;
    m_attackTimer = 0.0f;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 攻撃アクションの実行
BehaviorState ExecuteAttackAction::ExecuteAction(Executor* executorContext) {
	// 安全チェック
    if (executorContext == nullptr) {
        std::cerr << "ERROR: ExecuteAttack - Executor context is null."
            << std::endl;
        return BehaviorState::FAILURE;
    }

	// Conditionピンに接続されたノードから攻撃スキルの情報を取得
    std::string skillNodeId =
        executorContext->GetInputSourceNodeId(m_id.id, "Skill");
	// デバッグログ
    if (skillNodeId.empty()) {
        return BehaviorState::FAILURE;
    }
	// デバッグログ
    if (!skillNodeId.empty()) {
        const NodeInstance* skillNode =
            executorContext->GetNodeInstance(skillNodeId);
        if (skillNode) {
            float range = 1.0f;
            float damage = 10.0f;
            int skillId = 0;
            int score = 0;

            auto itRange = skillNode->properties.find("range");
            if (itRange != skillNode->properties.end())
                range = std::stof(itRange->second);

            auto itDamage = skillNode->properties.find("damage");
            if (itDamage != skillNode->properties.end())
                damage = std::stof(itDamage->second);

            auto itSkillId = skillNode->properties.find("skillId");
            if (itSkillId != skillNode->properties.end())
                skillId = std::stoi(itSkillId->second);

            auto itScore = skillNode->properties.find("score");
            if (itScore != skillNode->properties.end())
                score = std::stoi(itScore->second);

            executorContext->SetNextAttackSkill(skillId, range, damage, score);
        }
    }

	// 指定されたボックス内にターゲットが存在するか確認
    if (!executorContext->HasTargetsInBox(m_targetBox)) {
        return BehaviorState::FAILURE;
    }

	// 攻撃開始
    if (!m_isAttacking) {
        m_isAttacking = true;
        executorContext->StartAttack(m_targetBox);
    }

	// 攻撃中はRUNNINGを返す
    if (executorContext->IsAttacking()) {
        return BehaviorState::RUNNING;
    }

    m_isAttacking = false;
    return BehaviorState::SUCCESS;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ゴールへのルートを見つけるアクションの実行
BehaviorState FindRouteToGoalNode::ExecuteAction(Executor* executorContext) {
	// 安全チェック
    if (!executorContext)
        return BehaviorState::FAILURE;

	// ゴールオブジェクトを検索
    std::list<Goal*> goals = ObjectManager::FindGameObjects<Goal>();
    if (goals.empty()) {
        return BehaviorState::FAILURE;
    }

	// 最初のゴールをターゲットに設定
    Goal* goal = goals.front();
    if (goal) {
        executorContext->SetManualTargetPosition(goal->Position());
        return BehaviorState::SUCCESS;
    }

    return BehaviorState::FAILURE;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 障害物を検出するアクションの初期化
void DetectObstacleNode::Initialize() { ActionNode::Initialize(); }
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 障害物を検出するアクションの実行
BehaviorState DetectObstacleNode::ExecuteAction(Executor* executorContext) {
    if (!executorContext)
        return BehaviorState::FAILURE;

    VECTOR3 start = executorContext->Position();
    start.y += 1.0f;

    MATRIX4X4 mat = executorContext->Matrix();
    VECTOR3 forward(
        mat._31, mat._32,
        mat._33);

    forward = XMVector3Normalize(forward);

    VECTOR3 end = start + (forward * m_checkDistance);

    MeshCollider::CollInfo info;
    if (executorContext->Raycast(start, end, &info)) {
        std::cout << "INFO: Obstacle Detected at distance: "
            << (info.hitPosition - start).Length() << std::endl;
        return BehaviorState::SUCCESS;
    }

    return BehaviorState::FAILURE;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ステージタスクを取得するアクションの実行
BehaviorState GetStageTaskNode::ExecuteAction(Executor* executorContext) {
	// 安全チェック
    if (!executorContext)
        return BehaviorState::FAILURE;

    return BehaviorState::SUCCESS;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 攻撃スキルを実行するアクションの実行
BehaviorState AttackSkillNode::ExecuteAction(Executor* executorContext) {
	// 安全チェック
    if (!executorContext)
        return BehaviorState::FAILURE;

    executorContext->SetNextAttackSkill(m_skillId, m_range, m_damage, m_score);
    return BehaviorState::SUCCESS;
}
//-----------------------------------------------------------------------------