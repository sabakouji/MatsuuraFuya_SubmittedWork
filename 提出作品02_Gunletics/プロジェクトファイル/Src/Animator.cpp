#include "Animator.h"
#include <fstream>

Animator::Animator(Object2D* obj)
{
	object = obj;
	animNum = 1;		// アニメーション要素数(初期値は２)
	animIdx = 0;		// アニメーションインデックス数(インデックス位置はＸ方向)
	animFrame = 0;		// アニメーションフレームカウント
	flashNum = 2;		// フラッシュアニメーション要素数(初期値は２)
	flashIdx = 0;		// フラッシュアニメーションインデックス数(インデックス位置はＹ方向)
	flashFrame = 0;		// フラッシュアニメーションフレームカウント
	waitTime = 10;		// アニメーションのカウント待ち時間
	finished = false;	// アニメーションが終了したか
	flashOn = false;	// フラッシュアニメーションをするかどうか
	dir = diUp;			// 上方向
	alpha = 1.0f;		// 透明度

	currentAnimStartXIdx = 0; // 静止画のXインデックス
	currentAnimNumFrames = 1; // 静止画は1フレーム
	currentAnimYRowIdx = 0;   // 静止画のY行インデックス
}

Animator::~Animator()
{
}

void Animator::Update()
{
	if (object == nullptr)
		return;

	if (currentAnimNumFrames > 1) // アニメーションが複数フレームの場合のみ
	{
		finished = false;
		animFrame += 1;

		if (animFrame >= waitTime) {
			animFrame = 0;
			animIdx++;
			// アニメーションの範囲内でループ
			if (animIdx >= currentAnimStartXIdx + currentAnimNumFrames) {
				animIdx = currentAnimStartXIdx; // 最初のフレームに戻る
				finished = true; // アニメーションの一周が終了
			}
		}
	}
	else { // 単一フレームのアニメーションの場合
		animIdx = currentAnimStartXIdx; // 開始フレームに固定
		animFrame = 0;
		finished = true;
	}

	// フラッシュアニメーションの更新（これは汎用なのでそのまま）
	if (flashOn && flashNum > 1)
	{
		flashFrame += 1;

		if (flashFrame >= waitTime / 3) {
			flashFrame = 0;
			flashIdx++;
			if (flashIdx >= flashNum) {
				flashIdx = 0;
			}
		}
	}

	SetAnimation();
}

void Animator::SetAnimationRange(int startXIdx, int numFrames, int yRowIdx, bool reverse)
{
	// アニメーション範囲が変わった場合のみリセット
	if (currentAnimStartXIdx != startXIdx ||
		currentAnimNumFrames != numFrames ||
		currentAnimYRowIdx != yRowIdx ||
		isreverse != reverse)
	{
		isreverse = reverse;
		currentAnimStartXIdx = startXIdx;
		currentAnimNumFrames = numFrames;
		currentAnimYRowIdx = yRowIdx;
		if (isreverse)
		{
			animIdx = startXIdx + numFrames - 1;
		}
		else
		{
			animIdx = startXIdx;
		}
		animFrame = 0;      // フレームカウントもリセット
		finished = false;   // 新しいアニメーションが開始した
	}
}

void Animator::SetWaitTime(int wtime)
{
	waitTime = wtime;
}
bool Animator::IsFinished()
{
	return finished;
}
void Animator::SetAnimation()
{
	// 透明度
	if (flashIdx == 0)
	{
		object->Sprite()->m_vDiffuse.w = alpha;
	}
	else {
		object->Sprite()->m_vDiffuse.w = alpha * 0.2f; // 透明に近くする
	}

	object->Sprite()->m_ofX = object->Sprite()->GetSrcWidth() * animIdx;
	object->Sprite()->m_ofY = object->Sprite()->GetSrcHeight() * currentAnimYRowIdx;
}
void Animator::ResetAnimation()
{
	animIdx = 0;
	animFrame = 0;
	SetAnimation();
}
void Animator::SetAnimNum(int ANum)
{
	animNum = ANum;
}
int Animator::GetAnimNum()
{
	return animNum;
}
void Animator::SetFlashNum(int FNum)
{
	flashNum = FNum;
}
int Animator::GetFlashNum()
{
	return flashNum;
}
void Animator::SetFlash()
{
	flashOn = true;
}
void Animator::SetFlashIdx(int idx)
{
	flashIdx = idx;
}
void Animator::ResetFlash()
{
	flashOn = false;
	flashIdx = 0;
	flashFrame = 0;
	SetAnimation();
}
void Animator::SetDir(Dir di)
{
	dir = di;
}
float Animator::GetAngle()
{
	float angle = 0;
	// 下方向を０度としたときの角度
	switch (dir)
	{
	case diUp:
		angle = 180;
		break;

	case diRight:
		angle = 270;
		break;

	case diDown:
		angle = 0;
		break;

	case diLeft:
		angle = 90;
		break;
	}
	return angle;
}
VECTOR2 Animator::GetAngleVector()
{
	VECTOR2 angle = VECTOR2(0,0);
	// 下方向を０度としたときの角度
	switch (dir)
	{
	case diUp:
		angle.y = -1;
		break;

	case diRight:
		angle.x = 1;
		break;

	case diDown:
		angle.y = 1;
		break;

	case diLeft:
		angle.x = -1;
		break;
	}
	return angle;
}