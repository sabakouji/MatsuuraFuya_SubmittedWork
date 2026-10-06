#pragma once
#include "GameObject.h"

const float Deg2Rad = XM_PI / 180.0f;
const float Rad2Deg = 180.0f / XM_PI;

/// <summary>
/// オブジェクトの座標関係の情報
/// </summary>
class TransformData {
public:
	VECTOR2 position;			// 位置
	VECTOR2 scale;				// 拡縮
	float   rotation;			// 回転角度
	VECTOR2 center;				// 中心位置。座標原点
	MATRIX4X4 drawMatrix;		// 描画用行列
	TransformData() {
		position = VECTOR2(0, 0);
		scale = VECTOR2(1, 1);
		rotation = 0;
		center = VECTOR2(0, 0);
	};
	/// <summary>
	/// 描画用行列の計算
	/// </summary>
	void CalcDrawMatrix();
};

/// <summary>
///  2Dオブジェクトのクラス
///  ( GameObject を継承 )
/// </summary>
class Object2D : public GameObject
{
public:
	Object2D() : sprite(nullptr) {}
	virtual ~Object2D() {}
	virtual void Start() override {}
	virtual void Update() override {}
	virtual void Draw() override;
	const TransformData& Transform() { return transform; }
	CSprite* Sprite() { return sprite; }

	/// <summary>
	/// スプライトオブジェクトを生成しオブジェクトとtransformに値を設定する
	/// </summary>
	/// <param name="image">スプライトイメージ</param>
	/// <param name="pattern">スプライトパターン</param>
	/// <returns>正常に設定できればtrue、できなければfalse</returns>
	virtual bool CreateSprite(CSpriteImage* image, const VECTOR4 pattern);

	/// <summary>
	/// スプライトオブジェクトを生成しオブジェクトとtransformに値を設定する
	/// </summary>
	/// <param name="image">スプライトイメージ</param>
	/// <param name="srcX">スプライトパターンの左上Ｘ座標</param>
	/// <param name="srcY">スプライトパターンの左上Ｙ座標</param>
	/// <param name="srcwidth">スプライトパターンの幅</param>
	/// <param name="srcheight">スプライトパターンの高さ</param>
	/// <returns>正常に設定できればtrue、できなければfalse</returns>
	virtual bool CreateSprite(CSpriteImage* image, const DWORD& srcX, const DWORD& srcY, const DWORD& srcwidth, const DWORD& srcheight);

	virtual void SetPosition(VECTOR2 pos) {
		transform.position = pos;
	}
	virtual void SetScale(VECTOR2 sca) {
		transform.scale = sca;
	}
	virtual void SetRotation(float rot) {
		transform.rotation = rot;
	}
	virtual void SetCenter(VECTOR2 center) {
		transform.center = center;
	}

protected:
	TransformData transform;
	CSprite* sprite;
};