#include "WeaponManager.h"
#include "EnemyManager.h"
#include "MapManager.h"
#include "EffectManager.h"

namespace {
	const float MoveSpeed = 0.3f;
	const float LifeDistance = 20.0f;
	const int   WeaponFireMax = 10;
	const int   AttackPoint = 200;
};

WeaponFireBall::WeaponFireBall()
{
	mesh = ObjectManager::FindGameObject<WeaponManager>()->MeshList("FireBall")->mesh;	  // WeaponManagerのメッシュを参照する
	farDistance = LifeDistance;		  // 生存距離を設定する
}

WeaponFireBall::~WeaponFireBall()
{
	for (EffectFire& ef : efList)
	{
		ObjectManager::Destroy(ef.efObj); 	// 火の玉を囲む炎の削除
	}
}

void WeaponFireBall::Start()
{
	transform.rotation = GetLookatRotateVector(transform.position, target);	  // 移動方向を設定する

	EffectManager* efm = ObjectManager::FindGameObject<EffectManager>();
	EffectFire ef = {};
	for (int i = 0; i < WeaponFireMax; i++)		// 火の玉を囲む炎
	{
		efList.push_back(ef);
		efList.back().efObj = efm->Spawn<EffectBillfire>();
		efList.back().offset.x = Randomf(-0.2f, 0.2f);
		efList.back().offset.y = Randomf(-0.2f, 0.2f);
		efList.back().offset.z = Randomf(-0.4f, 0.4f);
		efList.back().efObj->SetParent(this);
	}
}

void WeaponFireBall::Update()
{
	VECTOR3 positionOld = transform.position;

	VECTOR3 foward = VECTOR3(0, 0, MoveSpeed * 60 * SceneManager::DeltaTime());
	transform.position = XMVector3TransformCoord(foward, transform.matrix());

	EffectManager* efm = ObjectManager::FindGameObject<EffectManager>();

	MeshCollider::CollInfo coll;
	if (HitCheckLine(positionOld, transform.position, &coll, AttackPoint))  // 当たり判定と相手へのダメージセット
	{
		efm->Spawn<EffectBillboard>("Bom3", coll.hitPosition);	   // 爆発効果の発生
		//efm->Spawn<EffectParticle>( coll.hitPosition, coll.normal);		   //	パーティクルの発生
		DestroyMe();
	}
	else {
		// マップとの当たり判定
		MapManager* mm = ObjectManager::FindGameObject<MapManager>();
		if (mm != nullptr) {
			VECTOR3 hit, normal;
			if (mm->IsCollisionLay(positionOld, transform.position, hit, normal))
			{
				efm->Spawn<EffectBillboard>("Bom3", hit);
				//efm->Spawn<EffectParticle>( hit, normal);
				DestroyMe();
			}
		}
	}


	// 火の玉を囲む炎の更新
	for ( EffectFire &ef : efList)
	{
		ef.efObj->SetPosition(transform.position + ef.offset);
	}

	farDistance -= MoveSpeed * 60 * SceneManager::DeltaTime();
	if (farDistance <= 0)
	{
		DestroyMe();     	// 自分の削除
	}
}

void WeaponFireBall::Draw()
{
	VECTOR3 save = GameDevice()->m_vLightDir;
	GameDevice()->m_vLightDir = VECTOR3(0, 0, 0);  // 陰影のない描画にする

	Object3D::Draw();

	GameDevice()->m_vLightDir = save;		// 陰影のある描画に戻す
}
