#pragma once
#include "Object3D.h"
#include "Animator.h"
#include "EffectBase.h"

class WeaponBase : public Object3D
{
public:
	// オーナーのオブジェクト区分
	const enum OwnerID {
		ePC = 0x00000001,
		eNPC = 0x00000002,
		eENM = 0x00000004,
		eOTHER = 0,
	};

	struct EffectFire
	{
		EffectBase* efObj;
		VECTOR3     offset;
		EffectFire() {
			efObj = nullptr;
			offset = VECTOR3(0, 0, 0);
		}
	};

protected:
	struct meshstruct {
		std::string name;
		CFbxMesh* mesh;
		VECTOR3   offset;
		meshstruct() {
			mesh = nullptr;
			offset = VECTOR3(0, 0, 0);
		}
	};

public:
	WeaponBase();
	virtual ~WeaponBase();
	virtual void SetOwner(OwnerID inowner) { owner = inowner; }

	void SetTarget(VECTOR3 targetIn) { target = targetIn; }

	void SetActive(bool active);

	/// <summary>
	/// 武器のラインと対象となるオブジェクトとの当たり判定
	/// 当たったときは対象のダメージAddDamage()を設定
	/// </summary>
	/// <param name="positionOld">一つ前の位置（発射位置）</param>
	/// <param name="position">現在位置（到達位置）</param>
	/// <param name="coll">ヒットしたときに当たった位置の情報(Out)</param>
	/// <param name="AttackPoint">攻撃力</param>
	/// <returns>当たったときtrue</returns>
	bool HitCheckLine(VECTOR3 positionOld, VECTOR3 position, MeshCollider::CollInfo* coll, int AttackPoint);

protected:
	OwnerID   owner;
	VECTOR3   target;

};