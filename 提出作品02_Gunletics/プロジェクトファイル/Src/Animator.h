#pragma once
#include "Object2D.h"
#include <map>
#include "Sprite.h"

//
//	アクション・ＲＰＧ専用のアニメータークラス
//

/// <summary>
/// アニメーションクラス
/// </summary>
class Animator {
public:
	const enum Dir {
		diUp = 0,
		diRight = 1,
		diDown = 2,
		diLeft = 3,
		diIdle = 4
	};
public:
	Animator(Object2D* obj);
	~Animator();

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update();

	/// <summary>
	/// アニメーションスピード（待ち時間）の設定
	/// </summary>
	/// <param name="waitTime"></param>
	virtual void SetWaitTime(int waitTime);

	/// <summary>
	/// アニメーションのモーション数の設定
	/// </summary>
	/// <param name="ANum">モーション数。規定値は２</param>
	virtual void SetAnimNum(int ANum);

	/// <summary>
	/// アニメーションのモーション数を得る
	/// </summary>
	/// <returns>モーション数</returns>
	virtual int GetAnimNum();

	/// <summary>
	/// フラッシュアニメーションのモーション数の設定
	/// </summary>
	/// <param name="FNum">フラッシュモーション数。規定値は２</param>
	virtual void SetFlashNum(int FNum);

	/// <summary>
	/// フラッシュアニメーションのモーション数を得る
	/// </summary>
	/// <returns>モーション数</returns>
	virtual int GetFlashNum();

	/// <summary>
	/// このクラスメンバーのアニメーション情報をスプライトクラスに設定する
	/// </summary>
	/// <param name="ANum"></param>
	virtual void SetAnimation();

	/// <summary>
	/// このクラスメンバーのアニメーション情報をリセットする
	/// </summary>
	virtual void ResetAnimation();

	/// <summary>
	/// アニメーションが最終フレームまで行ったか
	/// (finishedの値を返す)
	/// </summary>
	/// <returns>最終フレームまで行ったらtrue</returns>
	virtual bool IsFinished();

	/// <summary>
	/// フラッシュアニメーションフラグflashOnを真trueにする
	/// </summary>
	virtual void SetFlash();

	/// <summary>
	/// フラッシュアニメーションインデックスを設定にする
	/// </summary>
	/// <param name="idx">フラッシュアニメーションインデックス</param>
	virtual void SetFlashIdx(int idx);

	/// <summary>
	/// フラッシュアニメーション関係の値を全てリセットする
	/// </summary>
	virtual void ResetFlash();

	/// <summary>
	/// 方向を設定する
	/// </summary>
	/// <param name="dir">方向</param>
	virtual void SetDir(Dir di);

	/// <summary>
	/// 方向を求める
	/// </summary>
	/// <returns>Dir</returns>
	virtual Dir GetDir() { return dir; }

	/// <summary>
	/// 方向を求める
	/// </summary>
	/// <returns>下方向を０度としたときの角度</returns>
	virtual float GetAngle();
	virtual VECTOR2 GetAngleVector();

	/// <summary>
	/// 透明度を設定する
	/// </summary>
	/// <param name="alp"></param>
	virtual void SetAlpha(float alp) { alpha = alp; }

	void SetAnimationRange(int startXIdx, int numFrames, int yRowIdx, bool reverse);

private:
	Object2D* object;	// オブジェクト
	int	animNum;		// アニメーション要素数(初期値は２)
	int	animIdx;		// アニメーションインデックス数(インデックス位置はＸ方向)
	int	animFrame;		// アニメーションフレームカウント
	int	flashNum;		// フラッシュアニメーション要素数(初期値は２)
	int	flashIdx;		// フラッシュアニメーションインデックス数(インデックス位置はＹ方向)
	int	flashFrame;		// フラッシュアニメーションフレームカウント
	int waitTime;		// アニメーションのカウント待ち時間
	bool finished;		// アニメーションが終了したか
	bool flashOn;		// フラッシュアニメーションをするかどうか
	Dir dir;			// キャラの向いている方向
	float alpha;		// 透明度(フラッシュで透明度を操作するため必要)
	bool isreverse;

	int currentAnimStartXIdx; // 現在アクティブなアニメーションの開始Xインデックス
	int currentAnimNumFrames; // 現在アクティブなアニメーションのフレーム数
	int currentAnimYRowIdx;   // 現在アクティブなアニメーションのY行インデックス
};