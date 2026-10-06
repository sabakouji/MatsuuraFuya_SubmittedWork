#include "EnemyManager.h"
#include "WeaponManager.h"
#include "Player.h"
#include "Executor.h"
#include "DataCarrier.h"

namespace {
	const float MoveSpeed = 0.02f;
	const float RotSpeed = 3.0f;
	const float ReachAngle = 120.0f;        // 視野
	const float ReachDistLimit = 20;        // 近接Reach限界値
	const float AttackDistLimit = 10;       // 攻撃接近限界値
	const int   MaxHitPoint = 1000;
	const float MaxFlashTime = 5;
	const float MaxIdleTime = 480;
};

EnemyGolem::EnemyGolem()
{
	animator = new Animator(); // インスタンスを作成

	mesh = ObjectManager::FindGameObject<EnemyManager>()->MeshList("Golem");	  // EnemyManagerのメッシュを参照する
	animator->SetModel(mesh); // このモデルでアニメーションする
	animator->Play(aRun);
	animator->SetPlaySpeed(1.0f);

	// トゥーンシェーディングと輪郭線を有効にする
	SetToonEnabled(true);
	SetOutline(true, VECTOR4(0.0f, 0.0f, 0.0f, 1.0f), 2.0f);

	meshCol = new MeshCollider();
	meshCol->MakeFromMesh(mesh, animator);

	state = stNormal;
	atcstate = atWalk;
	phase = phWait;
	hitPoint = MaxHitPoint;

	flashTimer = 0;

	aroundID = 0;
	targetPlayer = nullptr;

	RegisterEnemyBody(3.0f);  // ECS: 敵ボディを登録（押し合い半径=Colliderと同値）
}

EnemyGolem::~EnemyGolem()
{
	UnregisterEnemyBody();    // ECS: 敵ボディを解除
	SAFE_DELETE(meshCol);
}

SphereCollider EnemyGolem::Collider()
{
	// 少し小さめのバウンディングボールとする
	SphereCollider col;
	col.radius = 3.0f;
	col.center = transform.position + VECTOR3(0, col.radius, 0);
	return col;
}

void EnemyGolem::Update()
{
	SyncEnemyBodyIn();  // ECS: 現在位置をコンポーネントへ同期

	VECTOR3 positionOld = transform.position;

	switch (state) {
	case stFlash:
		flashTimer -= 60 * SceneManager::DeltaTime();
		if (flashTimer <= 0) state = stNormal;
		updateNormal();
		break;
	case stNormal:
		updateNormal();
		break;
	case stDamage:
		updateDamage();
		break;
	case stDead:
		updateDead();
		break;
	}

	// 敵同士の押し合い（ECS: EnemyCollisionSystem が算出した押し戻しを適用）
	ApplyEnemyBodyPush();

	// 重力・マップ接触（ECS: EnemyPhysicsSystemが積分した落下速度をOOPが適用）
	ApplyEnemyGravity(positionOld);

	animator->Update();	   // 2024.9.5
}

void EnemyGolem::updateNormal()
{
	// plyerが視野に入ったかのチェック
	if (atcstate != atReach && atcstate != atAttack)
	{
		// 自分の視野に入ったら、プレイヤーに向かってくる
		Executor* executor = ObjectManager::FindGameObject<Executor>();
		if (CheckReach(executor, ReachAngle, ReachDistLimit))
		{
			atcstate = atReach;	  // 視野に入った
			targetPlayer = executor;	// ターゲットとなるＰＣ
			animator->Play(aRun);
		}
		else {
			targetPlayer = nullptr;
		}
	}

	switch (atcstate) {
	case atIdle:
		updateNormalIdle();
		break;
	case atWalk:
		updateNormalWalk();
		break;
	case atReach:
		updateNormalReach();
		break;
	case atAttack:
		updateNormalAttack();
		break;
	}

}

void EnemyGolem::updateNormalIdle()
{
	idleTimer -= 60 * SceneManager::DeltaTime();
	if (idleTimer <= 0 || targetPlayer != nullptr )
	{
		animator->Play(aRun);
		atcstate = atWalk;
	}
}
void EnemyGolem::updateNormalWalk()
{
	VECTOR3 target = navigationMap[aroundID];
	if ( MoveToTarget(target, MoveSpeed, RotSpeed) )
	{
		aroundID = (aroundID + 1) % navigationMap.size();	 // 到達したら次の目標値に
		animator->Play(aIdle);
		atcstate = atIdle;
		idleTimer = MaxIdleTime;
	}
}

// Reach状態のとき
void EnemyGolem::updateNormalReach()
{
	if (targetPlayer == nullptr){	// ターゲットとなるＰＣが無いとき(普通はあり得ない)
		atcstate = atWalk;
		return;
	}

	VECTOR3 toPlayer = targetPlayer->Position() - transform.position;	   // 自分から見たプレイヤーの位置
	if (magnitude(toPlayer) < AttackDistLimit)	 // 攻撃距離に入ったら
	{
		animator->Play(aAttack1);
		phase = phWait;
		atcstate = atAttack;		// 攻撃に行く
	}
	else {
		// エリアの外に出たら、atcstateをwalkに変える
		if (toPlayer.Length() >= ReachDistLimit) {
			atcstate = atWalk;
		}
		MoveToTarget(targetPlayer->Position(), MoveSpeed, RotSpeed);   // プレイヤーに向かう
	}
}

void EnemyGolem::updateNormalAttack()
{
	if (targetPlayer == nullptr){  	// ターゲットとなるＰＣが無いとき(普通はあり得ない)
		atcstate = atWalk;
		animator->Play(aRun);
		return;
	}

	WeaponManager* wm = ObjectManager::FindGameObject<WeaponManager>();
	switch (phase)
	{
	case phWait:	  // 攻撃開始待ち
		if (animator->CurrentFrame() >= 40)	   // 攻撃アニメーションが攻撃態勢に入ったとき
		{
			phase = phAttack;
		}
		break;

	case phAttack:	  // 攻撃
		wm->SpawnMany<WeaponFireBall>(transform.position + VECTOR3(0, 12, 0), targetPlayer->Collider().center, WeaponBase::eENM);
		phase = phEnd;
		break;

	case phEnd:	     // 攻撃終了待ち
		if (animator->Finished())
		{
			animator->Play(aRun);
			phase = phWait;
			atcstate = atWalk;
		}
		break;
	}
}

void EnemyGolem::updateDamage()
{
	state = stFlash;
	flashTimer = MaxFlashTime;
}

void EnemyGolem::updateDead()
{
	animator->MergePlay(aDead);
	if (animator->Finished()) {
		DataCarrier* dc = ObjectManager::FindGameObject<DataCarrier>();
		dc->AddScore(MaxHitPoint/10);   // 最大ＨＰの1/10をスコアに加える

		DestroyMe();
	}
}
