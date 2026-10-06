#pragma once
#include "Object2D.h"
#include "EnemyBase.h"

#include "EnemyTord.h"
#include "EnemyWolf.h"
#include "EnemyWater.h"
#include "EnemyFox.h"
#include "StrudyBlock.h"
#include "PointTarget.h"
#include "BossEnemy.h"

/// <summary>
/// 敵の管理クラス
/// </summary>
class EnemyManager : public EnemyBase
{
public:
	EnemyManager(CSpriteImage* inimage);
	~EnemyManager();

	void Update() override;

	/// <summary>
	/// 敵を発生させる
	/// </summary>
	/// <typeparam name="C">敵クラス名</typeparam>
	/// <param name="pos">発生位置</param>
	/// <returns>発生できたときはオブジェクト。できないときはnullptr</returns>
	template<class C> C* Spawn(VECTOR2 pos)
	{
		C* obj = Instantiate<C>(image);
		if (obj == nullptr) return nullptr;
		obj->SetPos(pos);
		return obj;
	}

	/// <summary>
	/// 発生中の敵を全て削除する
	/// </summary>
	void DestroyEnemy();

	void SetUpdateEnabled(bool enabled);

private:
	CSpriteImage* image;
	int count;
	bool spawnDone;
};