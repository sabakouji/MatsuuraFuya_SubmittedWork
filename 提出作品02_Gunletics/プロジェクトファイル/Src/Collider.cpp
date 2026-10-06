#include "Collider.h"
#include "ObjectManager.h"

Collider::Collider(Object2D* iobj)
{
	obj = iobj;
}
Collider::~Collider()
{
}

bool Collider::IsCollision(Object2D* other)
{
	float ratio = 1.0f; // 接触判定比率。例えば、0.8にすると外側の20%は接触判定から除外される。

	// オブジェクトがあること
	if ( obj == nullptr)   return false;
	if ( other == nullptr ) return false;
	if( obj->Sprite() == nullptr )   return false;
	if( other->Sprite() == nullptr )  return false;

	// 衝突判定
	bool ret = false;
	DWORD  xd, yd, w1, w2, h1, h2;

	// オブジェクトが回転しているとき、角度に応じて縦幅と横幅を入れ替える。
	if (obj->Transform().rotation <= 45 || (obj->Transform().rotation >= 135 && obj->Transform().rotation <= 225) || obj->Transform().rotation >= 315)
	{
		w1 = obj->Sprite()->GetSrcWidth() * obj->Transform().scale.x;
		h1 = obj->Sprite()->GetSrcHeight() * obj->Transform().scale.x;
	}
	else {
		h1 = obj->Sprite()->GetSrcWidth() * obj->Transform().scale.x;
		w1 = obj->Sprite()->GetSrcHeight() * obj->Transform().scale.x;
	}
	if (other->Transform().rotation <= 45 || (other->Transform().rotation >= 135 && other->Transform().rotation <= 225) || other->Transform().rotation >= 315)
	{
		w2 = other->Sprite()->GetSrcWidth() * other->Transform().scale.x;
		h2 = other->Sprite()->GetSrcHeight() * other->Transform().scale.x;
	}
	else {
		h2 = other->Sprite()->GetSrcWidth() * other->Transform().scale.x;
		w2 = other->Sprite()->GetSrcHeight() * other->Transform().scale.x;
	}

	// 両方のオブジェクトの中心点の座標を求める。その中心点間の距離とオブジェクトの大きさで衝突判定を行う
	xd = abs((long)((obj->Transform().position.x - obj->Transform().center.x * obj->Transform().scale.x + w1 / 2)
					- (other->Transform().position.x - other->Transform().center.x * other->Transform().scale.x + w2 / 2)));     // 中心点間の距離Ｘ方向
	yd = abs((long)((obj->Transform().position.y - obj->Transform().center.y * obj->Transform().scale.x + h1 / 2)
					- (other->Transform().position.y - other->Transform().center.y * other->Transform().scale.y + h2 / 2)));     // 中心点間の距離Ｙ方向

	if ((xd < (w1 / 2 + w2 / 2) * ratio) &&     // 中心点間の距離がオブジェクトの大きさの1/2*ratioより小さければ接触している
		(yd < (h1 / 2 + h2 / 2) * ratio)) ret = true;

	return ret;
}