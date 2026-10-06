#include "WeaponBase.h"
#include "Player.h"
#include "EnemyManager.h"

WeaponBase::WeaponBase()
{
	owner = eOTHER;
	animator = nullptr;
	col = nullptr;
	state = stNormal;
	beDamaged = 0;
	atc = 0;
	option = 0;
}

// –Ú“I’n‚Ö‚ÌˆÚ“®
bool WeaponBase::TargetMove(VECTOR2 target, float speed, VECTOR2& velocity)
{
	float mvX, mvY, n;
	bool ret = false;

	if (transform.position.x == target.x && transform.position.y == target.y) {  // –Ú“I’n‚É’B‚µ‚½‚Æ‚«
		velocity.x = 0;
		velocity.y = 0;
		ret = true; // –Ú“I’n‚É’B‚µ‚½
	}
	else {    // –Ú“I’n‚Ö‚ÌˆÚ“®ˆ—

		mvX = target.x - transform.position.x;   // –Ú“I’n‚Ü‚Å‚Ì‚w•ûŒü‚Ì•ÏˆÊ
		mvY = target.y - transform.position.y;   // –Ú“I’n‚Ü‚Å‚Ì‚x•ûŒü‚Ì•ÏˆÊ

		if (abs(mvX) >= abs(mvY)) {   // ‚w•ûŒü‚Ì‹——£‚ª’·‚¢‚Æ‚«
			if (abs(mvX) < speed) {        // ‚P‰ñ‚ÌˆÚ“®—Ê‚æ‚è‹ßÚ‚µ‚Ä‚¢‚é‚Æ‚«
				velocity.x = mvX;
				velocity.y = mvY;
			}
			else {
				if (mvX >= 0) {
					velocity.x = speed; // ‚w•ûŒü‚ÌˆÚ“®—Ê‚ğspeed‚É‚·‚é
				}
				else {
					velocity.x = -speed; // ‚w•ûŒü‚ÌˆÚ“®—Ê‚ğ-speed‚É‚·‚é
				}
				n = abs(mvX / speed);
				velocity.y = floor(mvY / n); // ‚w•ûŒü‚ÌˆÚ“®—Ê‚É‡‚í‚¹‚ÄA‚x•ûŒü‚ÌˆÚ“®—Ê‚ğİ’è‚·‚é
			}
		}
		else {                                // ‚x•ûŒü‚Ì‹——£‚ª’·‚¢‚Æ‚«
			if (abs(mvY) < speed) {        // ‚P‰ñ‚ÌˆÚ“®—Ê‚æ‚è‹ßÚ‚µ‚Ä‚¢‚é‚Æ
				velocity.x = mvX;
				velocity.y = mvY;
			}
			else {
				if (mvY >= 0) {
					velocity.y = speed; // ‚x•ûŒü‚ÌˆÚ“®—Ê‚ğspeed‚É‚·‚é
				}
				else {
					velocity.y = -speed; // ‚x•ûŒü‚ÌˆÚ“®—Ê‚ğ-speed‚É‚·‚é
				}
				n = abs(mvY / speed);
				velocity.x = floor(mvX / n); // ‚x•ûŒü‚ÌˆÚ“®—Ê‚É‡‚í‚¹‚ÄA‚w•ûŒü‚ÌˆÚ“®—Ê‚ğİ’è‚·‚é
			}
		}
		ret = false;    // ‚Ü‚¾–Ú“I’n‚É’B‚µ‚Ä‚¢‚È‚¢
	}
	return ret;
}

// ‚ ‚½‚è”»’è
bool WeaponBase::HitCheck()
{
	bool hit = false;
	bool linehit = false;

	if (col == nullptr)
		return false;	   // ƒRƒŠƒWƒ‡ƒ“‚ªİ’è‚³‚ê‚Ä‚¢‚È‚¢‚Æ‚«

	if (owner & ePC)   			// ‚o‚b‚ª”­Ë‚µ‚½’e
	{
		EnemyBase* other = nullptr;
		hit = col->Hitcheck<EnemyBase>(other);	// ‘S“G‚Æ‚Ì‚ ‚½‚è”»’è
		// “G‚É“–‚½‚Á‚Ä‚¢‚é‚Æ‚«
		if (hit && other != nullptr && other->IsNormal())
		{

			other->SetBeDamaged(atc);
		}
	}
	else if (owner & eENM)	   		// “G‚ª”­Ë‚µ‚½’e
	{
		Player* other = nullptr;
		hit = col->Hitcheck<Player>(other);	// ‚o‚b‚Æ‚Ì‚ ‚½‚è”»’è
		// ‚o‚b‚É“–‚½‚Á‚Ä‚¢‚é‚Æ‚«
		if (hit && other != nullptr && other->IsNormal())
		{
			other->SetBeDamaged(atc);
		}
	}
	return hit;
}