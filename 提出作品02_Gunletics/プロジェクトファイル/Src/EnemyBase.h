#pragma once
#include "Object2D.h"
#include "Collider.h"
#include "Animator.h"


/// <summary>
/// 敵オブジェクトの基底クラス
/// </summary>
class EnemyBase : public Object2D
{
public:
	EnemyBase();
	virtual ~EnemyBase() {};
	virtual void SetPos(VECTOR2 pos){ transform.position = pos;	}
	virtual void SetBeDamaged(int damaged){ beDamaged = damaged; state = stDamage;}
	virtual bool IsNormal() { return state == stNormal; }
	virtual int  Atc(){ return atc;	}

	void SetUpdateEnabled(bool enabled)
	{
		updateEnabled = enabled;
	}

	bool IsUpdateEnabled()const
	{
		return updateEnabled;
	}

	/// <summary>
	/// 目的地（targetX,Y）への移動処理
	/// </summary>
	/// <param name="target">目的地</param>
	/// <param name="speed">一回の移動量</param>
	/// <param name="velocity">移動量を設定する＆（Ｏｕｔ）</param>
	/// <returns>true：目的地に達した　　false:まだ目的地に達していない</returns>
	virtual bool TargetMove(VECTOR2 target, float speed, VECTOR2& velocity);

	/// <summary>
	/// PCとのあたり判定
	/// 当たった相手のstateをstDamageにし、beDamageに攻撃力をセットする
	/// </summary>
	/// <returns>当たったらtrue、当たらなかったらfalse</returns>
	virtual bool HitCheck();

protected:
	const enum State {
		stNormal = 0,
		stDamage,
		stDead,
		stFlash,
	};
	const enum AtcState {
		atNone = 0,
		atWait,
		atShot,
		atBright,
		atCannon
	};
	State state;
	AtcState atcstate;

	Animator* animator;
	Collider* col;
	int hp;
	int atc;
	int beDamaged;
	bool updateEnabled = true;
};