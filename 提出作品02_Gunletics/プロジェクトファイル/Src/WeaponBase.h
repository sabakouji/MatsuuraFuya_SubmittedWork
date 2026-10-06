#pragma once
#include "Object2D.h"
#include "Collider.h"
#include "Animator.h"

/// <summary>
/// 武器の基底クラス
/// </summary>
class WeaponBase : public Object2D
{
public:
	// オーナーのオブジェクト区分
	const enum OwnerID {
		ePC = 0x00000001,
		eNPC = 0x00000002,
		eENM = 0x00000004,
		eOTHER = 0,
	};
public:
	WeaponBase();
	virtual ~WeaponBase() {}
	virtual void SetOwner(OwnerID inowner){owner = inowner; }
	virtual void SetPos(VECTOR2 pos) { transform.position = pos; }
	virtual void SetRot(float rot) { transform.rotation = rot; }
	virtual void SetBedamaged(int damaged) { beDamaged = damaged; state = stDamage; }
	virtual bool IsNormal() { return state == stNormal; }
	virtual void SetOption(int opt) { option = opt; }
	virtual int Atc(){return atc;}

	/// <summary>
	/// 目的地（targetX,Y）への移動処理
	/// </summary>
	/// <param name="target">目的地</param>
	/// <param name="speed">一回の移動量(スピード)</param>
	/// <param name="velocity">実移動ベクトルを設定する＆（Ｏｕｔ）</param>
	/// <returns>目的地に達したらtrue、まだ目的地に達していないときはfalse</returns>
	virtual bool TargetMove(VECTOR2 target, float speed, VECTOR2& velocity);

	/// <summary>
	/// ＰＣや敵とのあたり判定
	/// 当たった相手のstateをstDamageにし、beDamageに攻撃力をセットする
	/// </summary>
	/// <returns>当たったらtrue、当たらなかったらfalse</returns>
	virtual bool HitCheck();

	void SetWeaponUpdateEnabled(bool Wenabled)
	{
		WeaponupdateEnabled = Wenabled;
	}

	bool IsWeaponUpdateEnabled()const
	{
		return WeaponupdateEnabled;
	}

protected:	
	const enum State {
		stNormal = 0,
		stDamage,
		stDead,
		stFlash,
		stAnchored,
		stRewinding
	};
	State     state;

	OwnerID   owner;
	Animator* animator;
	Collider* col;
	int       atc;
	int       beDamaged;
	int       option;
	bool WeaponupdateEnabled = true;
};