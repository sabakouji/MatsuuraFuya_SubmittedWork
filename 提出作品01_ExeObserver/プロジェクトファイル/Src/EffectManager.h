#pragma once
#include "ECS/World.h"
#include "Animator.h"

#include "EffectBase.h"
#include <string>
#include <list>

#include "EffectBillboard.h"
#include "EffectBillfire.h"


class EffectManager : public EffectBase
{
public:
	EffectManager();
	~EffectManager();

	void Draw() override;     // ECSパーティクルプールの一括描画（RenderSystem相当）

	// 命中位置 pos に、normal方向中心の半球状にECSパーティクルをバースト生成する。
	// 旧 Spawn<EffectParticle>(pos, normal) のECS置き換え。
	void EmitParticleBurst(VECTOR3 pos, VECTOR3 normal);

	/// <summary>
	/// 効果の発生処理
	/// </summary>
	/// <typeparam name="C">効果クラス名</typeparam>
	/// <param name="str">エフェクト名</param>
	/// <returns>発生できたときはオブジェクト,できないときはnullptr</returns>
	template<class C> C* Spawn(std::string str = "")
	{
		C* obj = Instantiate<C>();
		if (obj == nullptr)  return nullptr;
		obj->SetEffectName(str);
		return obj;
	}

	/// <summary>
	/// 効果の発生処理
	/// </summary>
	/// <typeparam name="C">効果クラス名</typeparam>
	/// <param name="startIn">発生位置</param>
	/// <returns>発生できたときはオブジェクト,できないときはnullptr</returns>
	template<class C> C* Spawn(VECTOR3 startIn)
	{
		C* obj = Instantiate<C>();
		if (obj == nullptr)  return nullptr;
		obj->SetPosition(startIn);
		obj->SetEffectName("");
		return obj;
	}

	/// <summary>
	/// 効果の発生処理
	/// </summary>
	/// <typeparam name="C">効果クラス名</typeparam>
	/// <param name="str">エフェクト名</param>
	/// <param name="startIn">発生位置</param>
	/// <returns>発生できたときはオブジェクト,できないときはnullptr</returns>
	template<class C> C* Spawn(std::string str, VECTOR3 startIn)
	{
		C* obj = Instantiate<C>();
		if (obj == nullptr)  return nullptr;
		obj->SetPosition(startIn);
		obj->SetEffectName(str);
		return obj;
	}

	/// <summary>
	/// 効果の発生処理
	/// </summary>
	/// <typeparam name="C">効果クラス名</typeparam>
	/// <param name="startIn">発生位置</param>
	/// <param name="normalIn">法線</param>
	/// <returns>発生できたときはオブジェクト,できないときはnullptr</returns>
	template<class C> C* Spawn( VECTOR3 startIn, VECTOR3 normalIn)
	{
		C* obj = Instantiate<C>();
		if (obj == nullptr)  return nullptr;
		obj->SetPosition(startIn);
		obj->SetNormal(normalIn);
		obj->SetEffectName("");
		return obj;
	}

	/// <summary>
	/// 効果の発生処理
	/// </summary>
	/// <typeparam name="C">効果クラス名</typeparam>
	/// <param name="str">エフェクト名</param>
	/// <param name="startIn">発生位置</param>
	/// <param name="normalIn">法線</param>
	/// <returns>発生できたときはオブジェクト,できないときはnullptr</returns>
	template<class C> C* Spawn(std::string str, VECTOR3 startIn, VECTOR3 normalIn)
	{
		C* obj = Instantiate<C>();
		if (obj == nullptr)  return nullptr;
		obj->SetPosition(startIn);
		obj->SetNormal(normalIn);
		obj->SetEffectName(str);
		return obj;
	}

	BILLBOARDBASE* BillboardList(std::string str);
	PARTICLEBASE*  ParticleList(std::string str);

	// =========================================================================
	// ECSパーティクルスポーン（ParticleSystem で一括処理）。public化し外部から発生可能に。
	// posIn    : 生成位置
	// velIn    : 初期速度
	// lifetime : 寿命（秒）
	// size     : パーティクルサイズ
	// texId    : テクスチャID（0=sparklen3）
	// 戻り値   : 生成したECSのEntityID
	// =========================================================================
	EntityID SpawnParticle(VECTOR3 posIn, VECTOR3 velIn,
	                       float lifetime = 1.0f, float size = 0.5f, int texId = 0)
	{
		World& world = World::GetInstance();
		EntityID id  = world.CreateEntity();

		auto& p      = world.AddComponent<ParticleComponent>(id);
		p.position   = posIn;
		p.velocity   = velIn;
		p.lifetime   = lifetime;
		p.maxLife    = lifetime;
		p.size       = size;
		p.texId      = texId;
		p.alpha      = 1.0f;
		p.alive      = true;

		return id;
	}

private:
	std::list<BILLBOARDBASE> billboardList;
	std::list<PARTICLEBASE>  particleList;
};