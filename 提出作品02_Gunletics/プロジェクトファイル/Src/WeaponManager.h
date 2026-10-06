#pragma once
#include "Object2D.h"
#include "Animator.h"
#include "WeaponBase.h"
#include "EnemyWeapon.h"
#include "WeaponPenetrate.h"
#include "WeaponShot.h"
#include "EnemyWeapon.h"
#include "WeaponGrenade.h"
#include "grapplinghook.h"

/// <summary>
/// 武器の管理クラス
/// </summary>
class WeaponManager : public WeaponBase
{
public:
	WeaponManager(CSpriteImage* inimage);
	~WeaponManager();
	void Start() override;
	void Update() override;

	/// <summary>
	/// 武器の発生処理
	/// </summary>
	/// <typeparam name="C">武器クラス名</typeparam>
	/// <param name="pos">発生位置</param>
	/// <param name="rot">発生角度</param>
	/// <param name="owner">発生オーナー</param>
	/// <returns>発生できたときはオブジェクト,できないときはnullptr</returns>
	
	template<class C> C* Spawn(VECTOR2 pos, float rot, OwnerID owner)
	{
		C* obj = Instantiate<C>(image);
		if (obj == nullptr)  return nullptr;
		obj->SetPos(pos);
		obj->SetRot(rot);
		obj->SetOwner(owner);
		return obj;
	}
	
	/// <summary>
	/// 武器の発生処理
	/// </summary>
	/// <typeparam name="C">武器クラス名</typeparam>
	/// <param name="pos">発生位置</param>
	/// <param name="rot">発生角度</param>
	/// <param name="opt">オプション</param>
	/// <param name="owner">発生オーナー</param>
	/// <returns>発生できたときはオブジェクト,できないときはnullptr</returns>
	
	template<class C> C* Spawn(VECTOR2 pos, float rot, int opt, OwnerID owner)
	{
		C* obj = Instantiate<C>(image);
		if (obj == nullptr)  return nullptr;
		obj->SetPos(pos);
		obj->SetRot(rot);
		obj->SetOption(opt);
		obj->SetOwner(owner);
		return obj;
	}
	/*
	/// <summary>
	/// 武器の発生処理 (grapplinghook 専用)
	/// grapplinghook は2つの CSpriteImage* を必要とするため、専用のオーバーロードを用意
	/// </summary>
	grapplinghook* SpawnGrapplingHook(VECTOR2 pos, float rot, OwnerID owner, CSpriteImage* ropeImage)
	{
		grapplinghook* obj = new grapplinghook(image, ropeImage); 
		if (obj == nullptr) return nullptr;
		obj->SetPos(pos);
		obj->SetRot(rot);
		obj->SetOwner(owner);
		return obj;
	}
	*/
	/// <summary>
	/// 発生中の武器を全て削除する
	/// </summary>
	void DestroyWeapon();
	void SetWeaponUpdateEnabled(bool Wenabled);

	CSpriteImage* GetImage() { return image; };

 private:
	CSpriteImage* image;
};