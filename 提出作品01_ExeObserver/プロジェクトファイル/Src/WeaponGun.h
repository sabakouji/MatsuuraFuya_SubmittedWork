#pragma once
#include "WeaponBase.h"
#include "Animator.h"


class WeaponGun : public WeaponBase {
public:
	WeaponGun();
	~WeaponGun();
	void Update() override;
	void Draw() override;
	void Start() override;

	/// <summary>
	/// 武器銃オブジェクトに各種設定値を設定する
	/// </summary>
	/// <param name="name">メッシュ名。メッシュがないときは""</param>
	/// <param name="meshNoIn">メッシュ番号</param>
	/// <param name="boneNoIn">ボーン番号</param>
	/// <param name="offsetIn">オフセット</param>
	/// <param name="posIn">移動調整値</param>
	/// <param name="rotIn">回転調整値</param>
	void SetWeaponGun(std::string name, int meshNoIn, int boneNoIn, VECTOR3 offsetIn = VECTOR3(0,0,0),
											VECTOR3 posIn=VECTOR3(0,0,0), VECTOR3 rotIn=VECTOR3(0,0,0));

	/// <summary>
	/// 弾を発射する
	/// </summary>
	/// <param name="owner">オーナー</param>
	/// <returns>発射できたらtrue</returns>
	bool ShotBullet(WeaponBase::OwnerID owner);

	/// <summary>
	/// レーザーを発射する
	/// </summary>
	/// <param name="owner">オーナー</param>
	/// <returns>発射できたらtrue</returns>
	bool ShotLaser(WeaponBase::OwnerID owner);

private:
	int  meshNo;
	int  boneNo;
	MATRIX4X4 adjustMatrix;
	MATRIX4X4 worldMatrix;
	VECTOR3 offset;
	VECTOR3 startPos;
	float bulletTimer;
};