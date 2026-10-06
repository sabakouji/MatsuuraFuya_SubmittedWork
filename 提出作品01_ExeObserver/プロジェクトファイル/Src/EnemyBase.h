#pragma once
#include "Object3D.h"
#include "Animator.h"
#include "ECS/World.h"

class EnemyBase : public Object3D
{
public:
	EnemyBase();
	virtual ~EnemyBase();

	void MakeNavigationMap(std::vector<VECTOR3> nvIn);
	SphereCollider Collider() override;
	bool MoveToTarget(VECTOR3 target, float speed = 0.01f, float rotSpeed = 3.0f);
	bool CheckReach(Object3D* object, float angle, float ReachDistLimit);
	void AddDamage(float damage, VECTOR3 pPos);

protected:
	// === ECS（段階的ハイブリッド）: 敵ボディのエンティティ管理 =================
	// 具象敵（Golem/Mecha）の ctor で Register、dtor/破棄時に Unregister する。
	// EnemyManager 自身は敵ではないため登録しない。
	void RegisterEnemyBody(float radius);  // エンティティ生成＋EnemyBodyComponent付与
	void UnregisterEnemyBody();            // エンティティ破棄
	void SyncEnemyBodyIn();                // OOP→ECS: 現在位置・半径をコンポーネントへ書き込む
	void ApplyEnemyBodyPush();             // ECS→OOP: Systemが算出した押し戻し量を位置へ適用（適用後0クリア）
	void ApplyEnemyGravity(VECTOR3 positionOld); // ECS→OOP: 落下速度を適用＋マップ衝突解決（接地で速度0）
	EnemyBodyComponent* EnemyBody();       // 自分のコンポーネント取得（未登録時 nullptr）

	EntityID m_enemyEntity = 0;
	bool     m_hasEnemyEntity = false;

protected:
	enum AnimID {
		aIdle = 0,
		aRun = 1,
		aDead,
		aAttack1
	};

	enum State {
		stNormal = 0, // 通常状態(巡回)
		stDamage,     // ダメージ
		stDead,       // 死亡
		stFlash
	};
	enum AtcState {
		atIdle = 0,
		atWalk,
		atReach,      // 追いかける
		atAttack
	};

	State state;
	AtcState atcstate;

	std::vector<VECTOR3> navigationMap;
	int hitPoint;
	float flashTimer; // 無敵時間
	float idleTimer;  // アイドル時間
};