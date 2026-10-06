#include "EnemyBase.h"
#include "Player.h"
#include "StrudyBlock.h"

EnemyBase::EnemyBase()
{
	state = stNormal;
	atcstate = atWait;
	animator = nullptr;
	col = nullptr;
	hp = 0;
	beDamaged = 0;
	atc = 0;
}

bool EnemyBase::TargetMove(VECTOR2 target, float speed, VECTOR2& velocity)
{
	float mvX, mvY, n;
	bool ret = false;

	if (transform.position.x == target.x && transform.position.y == target.y) {  // 目的地に達したとき
		velocity.x = 0;
		velocity.y = 0;
		ret = true; // 目的地に達した
	}
	else {    // 目的地への移動処理

		mvX = target.x - transform.position.x;   // 目的地までのＸ方向の変位
		mvY = target.y - transform.position.y;   // 目的地までのＹ方向の変位

		if (abs(mvX) >= abs(mvY)) {   // Ｘ方向の距離が長いとき
			if (abs(mvX) < speed) {        // １回の移動量より近接しているとき
				velocity.x = mvX;
				velocity.y = mvY;
			}
			else {
				if (mvX >= 0) {
					velocity.x = speed; // Ｘ方向の移動量をspeedにする
				}
				else {
					velocity.x = -speed; // Ｘ方向の移動量を-speedにする
				}
				n = abs(mvX / speed);
				velocity.y = floor(mvY / n); // Ｘ方向の移動量に合わせて、Ｙ方向の移動量を設定する
			}
		}
		else {                                // Ｙ方向の距離が長いとき
			if (abs(mvY) < speed) {        // １回の移動量より近接していると
				velocity.x = mvX;
				velocity.y = mvY;
			}
			else {
				if (mvY >= 0) {
					velocity.y = speed; // Ｙ方向の移動量をspeedにする
				}
				else {
					velocity.y = -speed; // Ｙ方向の移動量を-speedにする
				}
				n = abs(mvY / speed);
				velocity.x = floor(mvX / n); // Ｙ方向の移動量に合わせて、Ｘ方向の移動量を設定する
			}
		}
		ret = false;    // まだ目的地に達していない
	}
	return ret;
}

// あたり判定
bool EnemyBase::HitCheck()
{
	bool hit = false;

	if (col == nullptr)
		return false;

	if (dynamic_cast<StrudyBlock*>(this) != nullptr)
	{
		return false;
	}

	Player* other = nullptr;
	hit = col->Hitcheck<Player>(other);	 // ＰＣに当たっているとき
	if (hit && other != nullptr && other->IsNormal())	   // ＰＣがNormalの場合だけ判定
	{
		other->SetBeDamaged(atc);    // ＰＣへダメージをセット
		beDamaged = other->Atc();    // ＰＣから受けるダメージをセット
		state = stDamage;   // ダメージ処理へ
	}
	return hit;
}