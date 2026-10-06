#pragma once
#include "Object3D.h"
#include "TextReader.h"
#include "MapBase.h"

#include "MapStage.h"
#include "MapMove.h"
#include "MapSky.h"
#include "MapWave.h"
#include "MapTree.h"

class MapManager : public MapBase
{
public:
	MapManager();
	virtual ~MapManager();

	/// <summary>
	/// スクリプトテキストの指定によりマップを作成する
	/// </summary>
	/// <param name="txt">テキストリーダーオブジェクト</param>
	/// <param name="n">指定行位置</param>
	void MakeMap(TextReader* txt, int n );

	/// <summary>
	/// 通常マップ・移動マップと線分との当たり判定を行う
	/// 始点から一番近いポリゴンの当たり判定情報を返します
	/// ポリゴンの表面のみ判定し、裏面は判定しません
	/// </summary>
	/// <param name="startIn">線分の始点</param>
	/// <param name="endIn">線分の終点</param>
	/// <param name="vHit">当たった位置を入れる場所(Out)</param>
	/// <param name="vNormal">当たった位置の法線を入れる場所(Out)</param>
	/// <returns>当たっていればtrue</returns>
	bool IsCollisionLay(const VECTOR3& startIn, const VECTOR3& endIn, VECTOR3& hit, VECTOR3& normal);

	/// <summary>
	/// 通常マップ・移動マップと球体との当たり判定を行う
	/// 移動開始から一番近いポリゴンの当たり判定情報を返します
	/// </summary>
	/// <param name="startIn">移動開始点</param>
	/// <param name="endIn">移動終了点</param>
	/// <param name="fRadius">半径</param>
	/// <param name="vHit">当たった位置を入れる場所(Out)</param>
	/// <param name="vNormal">当たった位置の法線を入れる場所(Out)</param>
	/// <returns>当たっていればtrue</returns>
	bool IsCollisionSphere(const VECTOR3& startIn, const VECTOR3& endIn, const float& radius, VECTOR3& hit, VECTOR3& normal);

	/// <summary>
	/// 通常マップ・移動マップと球体の移動線分との重力を加味した当たり判定を行う
	/// 移動開始点から一番近いポリゴンで当たり判定を行います
	/// 当たっていた場合、球体がポリゴンの外に出るように移動終了点positionの値が変更されます
	/// </summary>
	/// <param name="positionOld">移動前点</param>
	/// <param name="position">移動後点(In/Out)</param>
	/// <param name="fRadius">半径</param>
	/// <returns>コリジョン判定の結果</returns>
	CollRet IsCollisionMoveGravity(const VECTOR3& positionOld, VECTOR3& position, float radius = 0.2f);

	/// 通常マップ・移動マップと球体の移動線分との重力を加味した当たり判定を行う
	/// 移動開始点から一番近いポリゴンで当たり判定を行います
	/// 当たっていた場合、球体がポリゴンの外に出るように移動終了点positionの値が変更されます
	/// </summary>
	/// <param name="positionOld">移動前点</param>
	/// <param name="position">移動後点(In/Out)</param>
	/// <param name="vHit">当たった位置を入れる場所(Out)</param>
	/// <param name="vNormal">当たった位置の法線を入れる場所(Out)</param>
	/// <param name="fRadius">半径</param>
	/// <returns>コリジョン判定の結果</returns>
	CollRet IsCollisionMoveGravity(const VECTOR3& positionOld, VECTOR3& position, VECTOR3& hit, VECTOR3& normal, float radius = 0.2f);

	/// <summary>
	/// 通常マップ・移動マップと球体の移動線分との当たり判定を行う
	/// 移動開始点から一番近いポリゴンで当たり判定を行います
	/// 当たっていた場合、球体がポリゴンの外に出るように移動終了点positionの値が変更されます
	/// </summary>
	/// <param name="positionOld">移動前点</param>
	/// <param name="position">移動後点(In/Out)</param>
	/// <param name="fRadius">半径</param>
	/// <returns>当たっていればtrue</returns>
	bool IsCollisionMove(const VECTOR3& positionOld, VECTOR3& position, float radius = 0.2f);

	/// <summary>
	/// 通常マップ・移動マップと球体の移動線分との当たり判定を行う
	/// 移動開始点から一番近いポリゴンの当たり判定情報を返します
	/// 当たっていた場合、球体がポリゴンの外に出るように移動終了点positionの値が変更されます
	/// </summary>
	/// <param name="positionOld">移動前点</param>
	/// <param name="position">移動後点(In/Out)</param>
	/// <param name="vHit">当たった位置を入れる場所(Out)</param>
	/// <param name="vNormal">当たった位置の法線を入れる場所(Out)</param>
	/// <param name="fRadius">半径</param>
	/// <returns>当たっていればtrue</returns>
	bool IsCollisionMove(const VECTOR3& positionOld, VECTOR3& position, VECTOR3& hit, VECTOR3& normal, float radius = 0.2f);

	CCollision* ColMesh() { return colMesh; }

private:
	CCollision* colMesh;	// 静的コリジョンをまとめたコリジョンメッシュ

};