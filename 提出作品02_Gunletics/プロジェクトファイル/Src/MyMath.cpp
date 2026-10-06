//-----------------------------------------------------------------------------
//
//  Direct3Dを利用するための汎用３Ｄライブラリ         ver 2.8        2019.12.30
// 
//	各種　数学系のライブラリ
//
//
//																MyMath.h
//
//-----------------------------------------------------------------------------
#include "MyMath.h"


//-----------------------------------------------------------------------------
//  注視点座標計算
//
//  視点から注視点を見るためのマトリックスを作成する
//
//  引数		
//  const VECTOR3& vHear	視点（自分の位置座標）
//  const VECTOR3& vLookat	注視点（目標とする位置の座標）
//
//	戻り値  MATRIX4X4 mOut
//			作成されたマトリックス
//
//-----------------------------------------------------------------------------
MATRIX4X4 GetLookatMatrix(const VECTOR3& vHear, const VECTOR3& vLookat)
{
	MATRIX4X4  matTemp, matTemp2, matRotX, matRotY, mOut;
	VECTOR3    vDist;
	float      mag_xz, mag_xyz;
	float      cos_x, sin_x, cos_y, sin_y;

	vDist = vLookat - vHear;
	mag_xyz = magnitude(vDist);
	vDist.y = 0.0f;
	mag_xz = magnitude(vDist);
	vDist = vLookat - vHear;

	if (1e-6f > mag_xyz) mag_xyz = 0.0001f;
	if (1e-6f > mag_xz)  mag_xz = 0.0001f;

	cos_y = vDist.z / mag_xz;
	sin_y = vDist.x / mag_xz;
	cos_x = mag_xz / mag_xyz;
	sin_x = -vDist.y / mag_xyz;

	// MAKE MATRIX
	matTemp = XMMatrixTranslation(vHear.x, vHear.y, vHear.z),

	matRotX = XMMatrixIdentity();
	matRotX._22 = cos_x;
	matRotX._23 = sin_x;
	matRotX._32 = -sin_x;
	matRotX._33 = cos_x;

	matRotY = XMMatrixIdentity();
	matRotY._11 = cos_y;
	matRotY._13 = -sin_y;
	matRotY._31 = sin_y;
	matRotY._33 = cos_y;

	matTemp2 = matRotX * matRotY;
	mOut     = matTemp2 * matTemp;

	return mOut;
}

//============================================================================
//   マトリックスから角度を求める
//    
//   角度を求める方法は、現マトリックスのＺ方向に１ｍ進めた位置が、現位置から見て
//   ＸＹＺの各方向に何度ずれているかで判定する
//    
//   const MATRIX4X4& mat	マトリックス
//   戻り値
//       VECTOR3 vOut
//============================================================================
VECTOR3 GetRotateVector(const MATRIX4X4& mat)
{
	MATRIX4X4	matObj, matObj2, matTemp;
	VECTOR3	vObj, vBase;
	VECTOR2 vP1, vP2;
	VECTOR3 vOut;

	matObj = mat;
	matObj._41 = 0.0f;
	matObj._42 = 0.0f;
	matObj._43 = 0.0f;

	matTemp = XMMatrixTranslation(0.0f, 0.0f, 1.0f),
	matObj2 = matTemp * matObj;


	// 角度のＸ成分を求める
	vBase.x = 0.0f;
	vBase.y = 0.0f;
	vBase.z = 1.0f;
	vObj.x = 0.0f;
	vObj.y = matObj2._42;
	vObj.z = matObj2._43;
	vOut.x = Angle(vBase, vObj) * 180 / 3.14159f;
	vP1.x = vBase.y;
	vP1.y = vBase.z;
	vP2.x = vObj.y;
	vP2.y = vObj.z;
	if (crossZ(vP1, vP2) < 0.0f) vOut.x *= -1.0f;

	// 角度のＹ成分を求める
	vBase.x = 0.0f;
	vBase.y = 0.0f;
	vBase.z = 1.0f;
	vObj.x = matObj2._41;
	vObj.y = 0.0f;
	vObj.z = matObj2._43;
	vOut.y = Angle(vBase, vObj) * 180 / 3.14159f;
	vP1.x = vBase.x;
	vP1.y = vBase.z;
	vP2.x = vObj.x;
	vP2.y = vObj.z;
	if (crossZ(vP1, vP2) >= 0.0f) vOut.y *= -1.0f;

	// 角度のＺ成分を求める
	matTemp = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	matObj2 = matTemp * matObj;

	vBase.x = 0.0f;
	vBase.y = 1.0f;
	vBase.z = 0.0f;
	vObj.x = matObj2._41;
	vObj.y = matObj2._42;
	vObj.z = 0.0f;
	vOut.z = Angle(vBase, vObj) * 180 / 3.14159f;
	vP1.x = vBase.x;
	vP1.y = vBase.y;
	vP2.x = vObj.x;
	vP2.y = vObj.y;
	if (crossZ(vP1, vP2) < 0.0f) vOut.z *= -1.0f;

	return vOut;
}

//============================================================================
//   マトリックスから角度を求める
//    
//   角度を求める方法は、現マトリックスのＺ方向に１ｍ進めた位置が、現位置から見て
//   ＸＹＺの各方向に何度ずれているかで判定する
//    
//   const MATRIX4X4& mNow	マトリックス
//   戻り値
//   VECTOR3 angle
//============================================================================
VECTOR3 GetRotateVector2(const MATRIX4X4& mNow)
{
	VECTOR3     angle;
	MATRIX4X4	mNext;
	VECTOR2     vec2, w;
	VECTOR3     vec3;

	mNext = XMMatrixTranslation(0.0f, 0.0f, 1.0f) * mNow;
	vec3 = GetPositionVector(mNext) - GetPositionVector(mNow);

	// Y回転
	vec2 = VECTOR2(vec3.x, vec3.z);
	w = XMVector2AngleBetweenVectors(vec2, VECTOR2(0,1));
	angle.y = w.x;

	// X回転
	vec2 = VECTOR2(vec3.y, vec3.z);
	w = XMVector2AngleBetweenVectors(vec2, VECTOR2(0,1));
	angle.x = w.y;

	// Z回転
	mNext = XMMatrixTranslation(0.0f, 1.0f, 0.0f) * mNow;
	vec3 = GetPositionVector(mNext) - GetPositionVector(mNow);
	vec2 = VECTOR2(vec3.x, vec3.y);
	w = XMVector2AngleBetweenVectors(vec2, VECTOR2(0,1));
	angle.z = w.x;

	angle.x = angle.x * 180.0f / XM_PI;
	angle.y = angle.y * 180.0f / XM_PI;
	angle.z = angle.z * 180.0f / XM_PI;

	return angle;
}

//============================================================================
//   マトリックスから角度を求める							// -- 2019.7.19
//    
//    角度を求める方法は、マトリックス中の角度成分のみを取りだして角度を
//    求めている
//    Ｙ回転マトリックス * Ｘ回転マトリックス * Ｚ回転マトリックスの場合は、
//    正確に各軸の角度が求まる
//    
//   引数
//   const MATRIX4X4& mWorld	マトリックス
//   戻り値
//   VECTOR3 angle
//============================================================================
VECTOR3 GetRotateVector3(const MATRIX4X4& mWorld)
{
	VECTOR3     angle;
	double      threshold = 0.001;

	if (fabsf(mWorld._23 - 1.0f) < threshold)  // Mat(2,3) sin(x)=1のとき
	{
		angle.x = XM_PI / 2;
		angle.y = 0;
		angle.z = atan2f(mWorld._12, mWorld._11);
	}
	else if (fabsf(mWorld._23 + 1.0f) < threshold)  // Mat(2,3) sin(x)=-1のとき
	{
		angle.x = -XM_PI / 2;
		angle.y = 0;
		angle.z = atan2f(mWorld._12, mWorld._11);
	}
	else {
		angle.x = asinf(mWorld._23);
		angle.y = atan2f(-mWorld._13, mWorld._33);
		angle.z = atan2f(-mWorld._21, mWorld._22);
	}

	// ラジアン角を度に変換
	angle.x = XMConvertToDegrees(angle.x);
	angle.y = XMConvertToDegrees(angle.y);
	angle.z = XMConvertToDegrees(angle.z);

	return angle;
}

//============================================================================
//   マトリックスから角度Ｘの行列を求める
//    
//   const MATRIX4X4& mat	マトリックス
//   戻り値
//       MATRIX4X4 mOut
//============================================================================
MATRIX4X4 GetRotateMatrixX(const MATRIX4X4& mat)
{
	VECTOR3 vRot = GetRotateVector(mat);
	MATRIX4X4 mOut;
	mOut = XMMatrixRotationX(XMConvertToRadians(vRot.x));

	return mOut;
}
//============================================================================
//   マトリックスから角度Ｙの行列を求める
//    
//   const MATRIX4X4& mat	マトリックス
//   戻り値
//       MATRIX4X4 mOut
//============================================================================
MATRIX4X4 GetRotateMatrixY(const MATRIX4X4& mat)
{
	VECTOR3 vRot = GetRotateVector(mat);
	MATRIX4X4 mOut;
	mOut = XMMatrixRotationY(XMConvertToRadians(vRot.y));

	return mOut;
}
//============================================================================
//   マトリックスから角度Ｚの行列を求める
//    
//   const MATRIX4X4& mat	マトリックス
//   戻り値
//       MATRIX4X4 mOut
//============================================================================
MATRIX4X4 GetRotateMatrixZ(const MATRIX4X4& mat)
{
	VECTOR3 vRot = GetRotateVector(mat);
	MATRIX4X4 mOut;
	mOut = XMMatrixRotationZ(XMConvertToRadians(vRot.z));

	return mOut;
}
//============================================================================
//   マトリックスから角度の行列を求める
//    
//    ・位置成分を取り除く
//    
//   const MATRIX4X4& mat	マトリックス
//   戻り値
//       MATRIX4X4 mOut
//============================================================================
MATRIX4X4 GetRotateMatrix(const MATRIX4X4& mat)
{
	MATRIX4X4 mOut;
	mOut = mat * XMMatrixTranslation(-mat._41, -mat._42, -mat._43);
	return mOut;
}
//============================================================================
//   マトリックスから位置を求める
//    
//   const MATRIX4X4& mat	マトリックス
//    
//   戻り値
//       VECTOR3
//============================================================================
VECTOR3 GetPositionVector(const MATRIX4X4& mat)
{
	return VECTOR3(mat._41, mat._42, mat._43);
}
//============================================================================
//   マトリックスから位置行列を求める
//    
//   const MATRIX4X4& mat	マトリックス
//    
//   戻り値
//       D3DMATRIX  mOut
//============================================================================
MATRIX4X4 GetPositionMatrix(const MATRIX4X4& mat)
{
	MATRIX4X4 mOut;
	mOut = XMMatrixTranslation(mat._41, mat._42, mat._43);

	return mOut;
}

//============================================================================
//   自マトリックスからターゲットを向くための角度を求める               // -- 2018.8.16
//    
//   自分のマトリックスとターゲット位置のベクトルを入力し
//   自分が今向いている角度とターゲット方向との角度の差分を求め
//   その値を０～３６０°の角度で返す
//    
//   引数
//       const MATRIX4X4&  mWorld	  自分のマトリックス
//       const VECTOR3&    vTarget  ターゲットとする位置
//   戻り値
//       VECTOR3    vOut     ターゲットを向くための０～３６０°の角度
//============================================================================
VECTOR3 GetTargetRotateVector(const MATRIX4X4& mWorld, const VECTOR3& vTarget)
{
	MATRIX4X4  mTemp;
	VECTOR3    vRa, vRb, vOut;

	mTemp = GetLookatMatrix(GetPositionVector(mWorld), vTarget);	// ターゲット方向を向くマトリックス
	vRa = GetRotateVector(mWorld);		// 自分の現在の向き
	vRb = GetRotateVector(mTemp);		// 自分から見たターゲットの方向
	vOut = vRb - vRa;					// 自分がターゲットを向くための角度

	// 作成した角度の正規化
	// 念のために角度を０～３６０°に正規化する
	if (vOut.x <  0)   vOut.x += 360;
	if (vOut.x >= 360) vOut.x -= 360;
	if (vOut.y <  0)   vOut.y += 360;
	if (vOut.y >= 360) vOut.y -= 360;
	if (vOut.z <  0)   vOut.z += 360;
	if (vOut.z >= 360) vOut.z -= 360;

	return vOut;
}

//============================================================================  // -- 2019.12.30
//   マトリックスから拡大縮小率を求める
//    
//   const MATRIX4X4& mat	マトリックス
//    
//   戻り値
//       VECTOR3  vScale : 現在の拡大縮小率 sx,sy,sz
//============================================================================
VECTOR3 GetScaleVector(const MATRIX4X4& mat)
{
	VECTOR3    vScale;

	vScale.x = sqrtf(mat._11*mat._11 + mat._21*mat._21 + mat._31*mat._31);
	vScale.y = sqrtf(mat._12*mat._12 + mat._22*mat._22 + mat._32*mat._32);
	vScale.z = sqrtf(mat._13*mat._13 + mat._23*mat._23 + mat._33*mat._33);

	return vScale;
}


//------------------------------------------------------------------------------
//
//　２次元・３次元ベクトル用汎用ライブラリ
//
//　２次元・３次元ベクトル用の各種汎用ライブラリを設定する
//　同様の働きをするＸＭライブラリなどもあるが、此処では全てを自作する
//
//------------------------------------------------------------------------------
//
// 値が０の時は、Ｆｌｏａｔの最小値にする（０を避ける）
//
FLOAT avoidZero(FLOAT inp)
{
	if (inp == 0.0f) {
		inp = FLT_EPSILON;
	}
	return inp;
}

//
// ２つの２Ｄベクトルの内積を計算する
//
FLOAT dot(const VECTOR2& vLhs, const VECTOR2& vRhs)
{
	return (vLhs.x*vRhs.x + vLhs.y*vRhs.y);
}
//
// ２つの３Ｄベクトルの内積を計算する
//
FLOAT dot(const VECTOR3& vLhs, const VECTOR3& vRhs)
{
	return (vLhs.x*vRhs.x + vLhs.y*vRhs.y + vLhs.z*vRhs.z);
}
//
// ２つの３Ｄベクトルの外積を計算する
//
VECTOR3 cross(const VECTOR3& vLhs, const VECTOR3& vRhs)
{
	return VECTOR3(vLhs.y*vRhs.z - vLhs.z*vRhs.y,
		vLhs.z*vRhs.x - vLhs.x*vRhs.z,
		vLhs.x*vRhs.y - vLhs.y*vRhs.x);
}
//
// ２つの２Ｄベクトルの外積を計算し、z要素を返す
// (z要素の値が正の場合、ベクトルvRhsはベクトルvLhsから反時計回りである。)
//
FLOAT crossZ(const VECTOR2& vLhs, const VECTOR2& vRhs)
{
	return vLhs.x*vRhs.y - vLhs.y*vRhs.x;
}

//
// ２Ｄベクトルの長さを求める
//
FLOAT magnitude(const VECTOR2& vLen)
{
	return sqrtf(vLen.x*vLen.x + vLen.y*vLen.y);
}

//
// ３Ｄベクトルの長さを求める
//
FLOAT magnitude(const VECTOR3& vLen)
{
	return sqrtf(vLen.x*vLen.x + vLen.y*vLen.y + vLen.z*vLen.z);
}

//
// ３Ｄベクトルの長さの２乗を求める
//
FLOAT magnitudeSQ(const VECTOR3& vLen)
{
	return vLen.x*vLen.x + vLen.y*vLen.y + vLen.z*vLen.z;
}
//
// ２Ｄベクトルをスケーリングした値を求める
//
VECTOR2 scaling(const VECTOR2& vVec, const FLOAT& Scale)
{
	return VECTOR2(vVec.x*Scale, vVec.y*Scale);
}
//
// ３Ｄベクトルをスケーリングした値を求める
//
VECTOR3 scaling(const VECTOR3& vVec, const FLOAT& Scale)
{
	return VECTOR3(vVec.x*Scale, vVec.y*Scale, vVec.z*Scale);
}

//
// ２Ｄベクトルの正規化したベクトルを求める
//
VECTOR2 normalize(const VECTOR2& vVec)
{
	return scaling(vVec, 1.0f / avoidZero(magnitude(vVec)));
}
//
// ３Ｄベクトルの正規化したベクトルを求める
//
VECTOR3 normalize(const VECTOR3& vVec)
{
	return scaling(vVec, 1.0f / avoidZero(magnitude(vVec)));
}

//
// ２つの３Ｄベクトルの角度を計算する
//
FLOAT Angle(const VECTOR3& v1, const VECTOR3& v2)
{
	return acosf(dot(v1, v2) / avoidZero(magnitude(v1)*magnitude(v2)));
}

//
// ３つの３Ｄベクトルの角度の和を計算する
// （この関数の戻り値が３６０°（２π）の時は、原点が３つの３Ｄベクトルの中にある）
//
FLOAT AddAngle(const VECTOR3& p1, const VECTOR3& p2, const VECTOR3& p3)
{
	FLOAT p12m, p23m, p31m;
	FLOAT w1, w2, w3;

	// 接触点が頂点に非常に近いとき（１ｍｍ以下）は、３Ｄベクトルの中とする  2005.10.1
	if (((p1.x <= 0.001f && p1.x >= -0.001f) &&
		(p1.y <= 0.001f && p1.y >= -0.001f) &&
		(p1.z <= 0.001f && p1.z >= -0.001f)) ||
		((p2.x <= 0.001f && p2.x >= -0.001f) &&
		(p2.y <= 0.001f && p2.y >= -0.001f) &&
			(p2.z <= 0.001f && p2.z >= -0.001f)) ||
			((p3.x <= 0.001f && p3.x >= -0.001f) &&
		(p3.y <= 0.001f && p3.y >= -0.001f) &&
				(p3.z <= 0.001f && p3.z >= -0.001f))) {
		return (2.0f * 3.14159f);
	}

	{
		FLOAT p1m = magnitude(p1),	// ３Ｄベクトルｐ１の絶対値（長さ）
			p2m = magnitude(p2),	// ３Ｄベクトルｐ２の絶対値（長さ）
			p3m = magnitude(p3);	// ３Ｄベクトルｐ３の絶対値（長さ）
		p12m = p1m * p2m;
		p23m = p2m * p3m;
		p31m = p3m * p1m;
	}

	// 訂正　2005.9.25
	//return acosf(dot(p1, p2) / avoidZero(p12m)) +		// ３Ｄベクトルｐ１とｐ２の角度
	//       acosf(dot(p2, p3) / avoidZero(p23m)) +		// ３Ｄベクトルｐ２とｐ３の角度
	//       acosf(dot(p3, p1) / avoidZero(p31m));		// ３Ｄベクトルｐ３とｐ１の角度

	w1 = dot(p1, p2) / avoidZero(p12m);		// ３Ｄベクトルｐ１とｐ２のｃｏｓ値
	if (w1 < -1.0f) w1 = -1.0f;
	if (w1 >  1.0f) w1 = 1.0f;

	w2 = dot(p2, p3) / avoidZero(p23m);		// ３Ｄベクトルｐ２とｐ３のｃｏｓ値
	if (w2 < -1.0f) w2 = -1.0f;
	if (w2 >  1.0f) w2 = 1.0f;

	w3 = dot(p3, p1) / avoidZero(p31m);		// ３Ｄベクトルｐ３とｐ１のｃｏｓ値
	if (w3 < -1.0f) w3 = -1.0f;
	if (w3 >  1.0f) w3 = 1.0f;

	return (acosf(w1) + acosf(w2) + acosf(w3));		// ３Ｄベクトルの角度の和

}
//
// ４つの３Ｄベクトルの角度の和を計算する
//
FLOAT AddAngle(const VECTOR3& p1, const VECTOR3& p2, const VECTOR3& p3, const VECTOR3& p4)
{
	FLOAT p12m, p23m, p34m, p41m;
	FLOAT w1, w2, w3, w4;

	// 接触点が頂点に非常に近いとき（１ｍｍ以下）は、４Ｄベクトルの中とする  2005.10.1
	if (((p1.x <= 0.001f && p1.x >= -0.001f) &&
		(p1.y <= 0.001f && p1.y >= -0.001f) &&
		(p1.z <= 0.001f && p1.z >= -0.001f)) ||
		((p2.x <= 0.001f && p2.x >= -0.001f) &&
		(p2.y <= 0.001f && p2.y >= -0.001f) &&
			(p2.z <= 0.001f && p2.z >= -0.001f)) ||
			((p3.x <= 0.001f && p3.x >= -0.001f) &&
		(p3.y <= 0.001f && p3.y >= -0.001f) &&
				(p3.z <= 0.001f && p3.z >= -0.001f)) ||
				((p4.x <= 0.001f && p4.x >= -0.001f) &&
		(p4.y <= 0.001f && p4.y >= -0.001f) &&
					(p4.z <= 0.001f && p4.z >= -0.001f))) {
		return (2.0f * 3.14159f);
	}
	{
		FLOAT p1m = magnitude(p1),
			p2m = magnitude(p2),
			p3m = magnitude(p3),
			p4m = magnitude(p4);
		p12m = p1m * p2m;
		p23m = p2m * p3m;
		p34m = p3m * p4m;
		p41m = p4m * p1m;
	}
	// 訂正　2005.9.25
	//return acosf(dot(p1, p2) / avoidZero(p12m)) +
	//       acosf(dot(p2, p3) / avoidZero(p23m)) +
	//       acosf(dot(p3, p4) / avoidZero(p34m)) +
	//       acosf(dot(p4, p1) / avoidZero(p41m));

	w1 = dot(p1, p2) / avoidZero(p12m);		// ４Ｄベクトルｐ１とｐ２のｃｏｓ値
	if (w1 < -1.0f) w1 = -1.0f;
	if (w1 >  1.0f) w1 = 1.0f;

	w2 = dot(p2, p3) / avoidZero(p23m);		// ４Ｄベクトルｐ２とｐ３のｃｏｓ値
	if (w2 < -1.0f) w2 = -1.0f;
	if (w2 >  1.0f) w2 = 1.0f;

	w3 = dot(p3, p4) / avoidZero(p34m);		// ４Ｄベクトルｐ３とｐ４のｃｏｓ値
	if (w3 < -1.0f) w3 = -1.0f;
	if (w3 >  1.0f) w3 = 1.0f;

	w4 = dot(p4, p1) / avoidZero(p41m);		// ４Ｄベクトルｐ４とｐ１のｃｏｓ値
	if (w4 < -1.0f) w4 = -1.0f;
	if (w4 >  1.0f) w4 = 1.0f;

	return (acosf(w1) + acosf(w2) + acosf(w3) + acosf(w4));		// ４Ｄベクトルの角度の和}
}

//-----------------------------------------------------------------------------
//  乱数の発生
//
//  int  min1  乱数上限値
//  int  max1  乱数下限値
//
//  戻り値　　int  乱数
//-----------------------------------------------------------------------------
int Random(int min1, int max1)
{
	static unsigned int cnt = 0;
	int range;

	if (cnt == 0) {
		srand((unsigned int)timeGetTime());
		cnt = 1;
	}
	range = max1 - min1 + 1;
	return((rand() % range) + min1);
}
