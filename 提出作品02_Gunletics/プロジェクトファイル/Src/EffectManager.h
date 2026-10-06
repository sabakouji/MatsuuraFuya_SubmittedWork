#pragma once
#include "Object2D.h"
#include "Animator.h"
#include "EffectBase.h"

#include "EffectBom.h"

/// <summary>
/// 効果の管理クラス
/// </summary>
class EffectManager : public EffectBase
{
public:
	EffectManager(CSpriteImage* image);
	~EffectManager();
	void Start() override;
	void Update() override;

	/// <summary>
	/// 効果を発生させる
	/// </summary>
	/// <typeparam name="C">効果クラス名</typeparam>
	/// <param name="pos">発生位置</param>
	/// <returns>発生できたときはオブジェクト,できないときはnullptr</returns>
	template<class C> C* Spawn(VECTOR2 pos)
	{
		C* obj = Instantiate<C>(image);
		if (obj == nullptr) return nullptr;
		obj->SetPos(pos);
		return obj;
	}

	/// <summary>
	/// Effectオブジェクトを全て削除する
	/// </summary>
	void DestroyEffect();

private:
	CSpriteImage* image;
};