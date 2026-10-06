#include "WeaponManager.h"
#include "EnemyManager.h"
#include "MapManager.h"
#include "EffectManager.h"
#include "AudioManager.h"

namespace {
	const float LifeDistance = 30.0f;
	const int   AttackPoint = 50;
};

WeaponLaser::WeaponLaser()
{
	mesh = ObjectManager::FindGameObject<WeaponManager>()->MeshList("Laser")->mesh;	  // WeaponManagerのメッシュを参照する
	farDistance = LifeDistance;		  // 生存距離を設定する
	AudioManager::Audio("SeLaser")->Play();
}

WeaponLaser::~WeaponLaser()
{
}

void WeaponLaser::Start()
{
	transform.rotation = GetLookatRotateVector(transform.position, target);	  // 移動方向を設定する
}

void WeaponLaser::Update()
{
	VECTOR3 foward = VECTOR3(0, 0, LifeDistance);
	VECTOR3 positionNew = XMVector3TransformCoord(foward, transform.matrix());	  // レーザーの先端位置

	EffectManager* efm = ObjectManager::FindGameObject<EffectManager>();

	MeshCollider::CollInfo coll;
	if (HitCheckLine(transform.position, positionNew, &coll, AttackPoint))  // 当たり判定と相手へのダメージセット
	{
		efm->EmitParticleBurst( coll.hitPosition, coll.normal);		   //	パーティクルの発生
	}
	else {
		// マップとの当たり判定
		MapManager* mm = ObjectManager::FindGameObject<MapManager>();
		if (mm != nullptr) {
			VECTOR3 hit, normal;
			if (mm->IsCollisionLay(transform.position, positionNew, hit, normal))
			{
				efm->EmitParticleBurst( hit, normal);
			}
		}
	}
}

void WeaponLaser::Draw()
{
	VECTOR3 save = GameDevice()->m_vLightDir;
	GameDevice()->m_vLightDir = VECTOR3(0, 0, 0);  // 陰影のない描画にする

	transform.scale = VECTOR3(1, 1, LifeDistance);	 // Z方向にLifeDistance倍引き延ばす

	Object3D::Draw();

	transform.scale = VECTOR3(1, 1, 1);	 	 // １倍に戻す

	GameDevice()->m_vLightDir = save;		// 陰影のある描画に戻す

	DestroyMe();     	// １回ですぐ消す
}
