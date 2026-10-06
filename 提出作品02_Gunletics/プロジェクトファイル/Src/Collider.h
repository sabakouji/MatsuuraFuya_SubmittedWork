#pragma once
#include "Object2D.h"
#include <map>
#include <string>
#include "Sprite.h"

/// <summary>
/// オブジェクトの接触判定クラス
/// </summary>
class Collider
{
public:
	Collider(Object2D* obj);
	~Collider();

	/// <summary>
	/// コリジョンチェック
	/// </summary>
	/// <param name="other">相手のオブジェクト</param>
	/// <returns>接触していればtrue、以外はfalse</returns>
	bool IsCollision(Object2D* other);

	/// <summary>
	/// あたり判定
	/// </summary>
	/// <typeparam name="C">対象とする相手のクラス名</typeparam>
	/// <param name="otherout">相手のオブジェクト</param>
	/// <returns>接触していればtrue、以外はfalse</returns>
	template <class C> bool Hitcheck(C* &otherout)
	{
		bool hit = false;

		// 対象とするクラスの管理リストを作成する
		const std::list<C*> otherlist = ObjectManager::FindGameObjects<C>();

		// 管理リストを探索し全ての相手とあたり判定を行う
		for (C* other : otherlist)
		{
			if (obj == other) continue;
			hit = IsCollision(other);
			if (hit) {
				otherout = other;
				break;
			}
		}
		return hit;
	}
private:
	Object2D* obj;
 };