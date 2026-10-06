#include "Player.h"
#include "GameMain.h"
#include "Sprite.h"
#include "EnemyManager.h"
#include "WeaponManager.h"
#include "ItemManager.h"
#include "EffectManager.h"
#include "ObjectManager.h"
#include "Map.h"
#include "DInput.h"
#include <chrono>

namespace {
	const int MaxNum = 0;
	const int MaxHp = 600;
	const int MaxMg = 10;
	const int MaxPnMg = 5;
	const int MaxGrMg = 3;
	const int MaxAtc = 600;
	const int OriginalMoveSpeed = 5;
	const float JumpSpeed = 17.5f;
	const float Reloadtime = 1.0f;
	const float waittime = 0.5f;
	const int Usemg = 1;
	const int Incmg = 2;
	const int FlashWaitTime = 30;
	const int DeadWaitTime = 120;
	const float GRAVITY = 0.8f;
	const VECTOR4 SrcPatternNormal = VECTOR4(0, 0, 63, 87);
	const VECTOR4 SrcPatternHead = VECTOR4(0, 257, 27, 22);
	const VECTOR4 SrcPatternGun = VECTOR4(0, 281, 59, 24);
	const float GroundFriction = 0.8f;
	const float AirFriction = 0.98f;
	const float AirControlAcceleration = 0.5f;
};

Player::Player(CSpriteImage* image) : hook(nullptr)
{
	CreateSprite(image, SrcPatternNormal);

	headsprite = new RenderComponent(image, SrcPatternHead);
	gunsprite = new RenderComponent(image, SrcPatternGun);

	animator = new Animator(this);
	animator->SetWaitTime(8);

	state = State::stNormal;
	atcstate = AtcState::atWalk;

	objMap = nullptr;
	weapon = nullptr;
	jumpTime = 0;
	jumpSpeed = VECTOR2(0, 0);
	MoveSpeed = OriginalMoveSpeed;
	wait = 0;
	shotWait = 0;
	num = MaxNum;
	hp = MaxHp;
	mg = MaxMg;
	pnmg = MaxPnMg;
	grmg = MaxGrMg;
	atc = MaxAtc;
	beDamaged = 0;
	Reloadmode = false;
	jumppossibility = true;
	StopState = false;
	shottrriger = false;
	isDead = false;
	maxhp = MaxHp;
	second = 0;
	tensecond = 0;
	minute = 0;
	stopState = false;
	gravity = GRAVITY;
	isgrappling = false;
	isokgrappling = true;
	onGround = false;
	iscame = false;

	havepoint = 0;

	m_pDI = new CDirectInput();

	point = m_pDI->GetMousePos();

	AddWeapon(Weapon_Shot);
}

Player::~Player()
{
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
	SAFE_DELETE(m_pDI);
}

void Player::SetBeDamaged(int damaged)
{ 
	beDamaged = damaged;
	state = stDamage;
}
float Player::HpdivMax()
{ 
	return (float)hp / MaxHp;
}
float Player::MpdivMax()
{ 
	return (float)mg / MaxMg;
}

int Player::MaxHP()
{
	return maxhp;
}

int Player::Getmg()
{
	int weaponID = GetCurrentWeapon();
	switch (weaponID)
	{
	case Weapon_Shot:
		mgsize = mg;
		break;
	case Weapon_Penetrate:
		mgsize = pnmg;
		break;
	case Weapon_Grenade:
		mgsize = grmg;
		break;
	}
	return mgsize;
}

void Player::SetHpforMax()
{
	hp = MaxHp;
}

void Player::SetDir(Animator::Dir dir)
{
	animator->SetDir(dir);
}

VECTOR2 Player::GetPlayerPos() const
{
	VECTOR2 scroll = GameDevice()->Scroll;

	VECTOR2 PlayerWorldPos = VECTOR2
	{
		static_cast<float>(position.x) + scroll.x,
		static_cast<float>(position.y) + scroll.y
	};

	return  PlayerWorldPos;
}

void Player::FalseGrappling()
{
	isgrappling = false;
	hook = nullptr;
}

float Player::GetBasegravity()
{
	return GRAVITY;
}

void Player::SetGrapplingHook(grapplinghook* sethook)
{
	hook = sethook;
}

void Player::DefalteGravity()
{
	gravity = GRAVITY;
}

void Player::Start()
{
	transform.position.x = 400;
	transform.position.y = 0;
	objMap = ObjectManager::FindGameObject<Map>();
	weapon = ObjectManager::FindGameObject<WeaponManager>();
}

void Player::Update()
{
	if (stopState)
	{
		return;
	}

	DataCarrier* data = ObjectManager::FindGameObject<DataCarrier>();
	frame++;
	if (frame > 60)
	{
		second++;
		frame = 0;
	}
	if (second > 9)
	{
		tensecond++;
		second = 0;
	}
	if (tensecond > 5)
	{
		minute++;
		tensecond = 0;
	}

	MapLine* pHitmapline = nullptr;
	CDirectInput* input = GameDevice()->m_pDI;
	position = transform.position;

	point = m_pDI->GetMousePos();
	headsprite->sidechange(rigthside);
	gunsprite->sidechange(rigthside);

	isgrappling = (hook != nullptr && hook->IsAnchored());
	isokgrappling = (hook != nullptr && hook->IsOnline());

	switch (state) {
	case State::stFlash:
		if (--wait <= 0) {
			state = stNormal;
			animator->ResetFlash();
		}
		updateNormal();
		break;
	case State::stNormal:
		updateNormal();
		break;
	case State::stDamage:
		updateDamage();
		break;
	case State::stDead:
		updateDead();
		break;
	case State::stStop:
		updateStop();
		break;
	}

	if (velocity.y < 0.0f || (hook && hook->IsGrappling()))
	{
		atcstate = AtcState::atJump;
	}

	switch (atcstate) {
	case AtcState::atWalk:
		onGround = true;
		updateWalk();
		if (!isgrappling) velocity.x *= GroundFriction;
		break;
	case AtcState::atJump:
		if (objMap->isCollisionMoveMap(this, velocity, pHitmapline))
		{
			if (pHitmapline && pHitmapline->Normal.y <= 0)
			{
				atcstate = AtcState::atWalk;
				jumpTime = 0;
				jumpSpeed = VECTOR2(0, 0);
			}
		}
		onGround = false;
		if (hook == nullptr)
		{
			updateJump();
		}
		break;
	}

	if (!StopState)
	{
		animator->Update();
	}

	if (!iscame)
	{
		if (objMap->isCollisionMoveMap(this, velocity, pHitmapline))
		{
			if (pHitmapline && pHitmapline->Normal.y <= 0)
			{
				atcstate = AtcState::atWalk;
				jumpTime = 0;
				jumpSpeed = VECTOR2(0, 0);
			}
		}
	}

	if (atcstate == AtcState::atJump)
	{
		if (!isgrappling)
		{
			velocity.y += gravity;
		}
	}

	transform.position += velocity;

	updateMouse();
	Reload();
	shotwaittime();
}

void Player::Draw()
{
	Object2D::Draw();

	if (rigthside)
	{
		headoffset = VECTOR2(transform.position.x - 2.0f, transform.position.y - 63.0f);
	}
	else
	{
		headoffset = VECTOR2(transform.position.x + 2.0f, transform.position.y - 63.0f);
	}

	headsprite->SetPosition(headoffset);

	headsprite->Draw();

	if (rigthside)
	{
		gunoffset = VECTOR2(transform.position.x, transform.position.y - 63.0f);
		pivotPos = VECTOR2(SrcPatternGun.z - SrcPatternGun.z / 8, SrcPatternGun.w / 3);
	}
	else
	{
		gunoffset = VECTOR2(transform.position.x, transform.position.y - 63.0f);
		pivotPos = VECTOR2(SrcPatternGun.z / 8, SrcPatternGun.w / 3);
	}
	gunsprite->SetCenter(pivotPos);
	gunsprite->SetPosition(gunoffset);
	gunsprite->Draw();
}

void Player::AddWeapon(int weponID)
{
	if (!HasWeapon(weponID))
	{
		weaponInventory.push_back(weponID);
	}
}

bool Player::HasWeapon(int weponID) const
{
	return std::find(weaponInventory.begin(), weaponInventory.end(), weponID) != weaponInventory.end();
}

void Player::switchWeapon()
{
	if (weaponInventory.empty())
	{
		return;
	}
	currentweaponIndex = (currentweaponIndex + 1) % weaponInventory.size();
}

int Player::GetCurrentWeapon() const
{
	if (weaponInventory.empty())
	{
		return -1;
	}
	return weaponInventory[currentweaponIndex];
}

void Player::updateMouse()
{
	scroll = GameDevice()->Scroll;

	mousePosWorld.x = static_cast<float>(point.x) + scroll.x;
	mousePosWorld.y = static_cast<float>(point.y) + scroll.y;

	offset.x = -transform.center.x + sprite->GetDestWidth() / 2.0f;
	offset.y = -transform.center.y + sprite->GetDestHeight() / 2.0f;

	matScale = XMMatrixScaling(transform.scale.x, transform.scale.y, 1.0f);
	matRot = XMMatrixRotationZ(XMConvertToRadians(transform.rotation));
	matTrans = XMMatrixTranslation(transform.position.x, transform.position.y, 0.0f);

	worldMat = matScale * matRot * matTrans;

	localVec = XMVectorSet(offset.x, offset.y, 0.0f, 1.0f);
	worldVec = XMVector4Transform(localVec, worldMat);

	XMStoreFloat3(&shotWorldPos, worldVec);

	dir.x = mousePosWorld.x - shotWorldPos.x;
	dir.y = mousePosWorld.y - shotWorldPos.y;
	shotangle = atan2f(dir.y, dir.x) * (180.0f / 3.14159265f);

	if (shotangle >= 90.0f || shotangle <= -90.0f)
	{
		rigthside = true;
	}
	else
	{
		rigthside = false;
	}

	finalAngle = shotangle;
	
	if (rigthside)
	{
		finalAngle = 180.0f - shotangle;
		
		if (finalAngle > 180.0f)
		{
			finalAngle -= 360.0f;
		}
		else if(finalAngle < -180.0f)
		{
			finalAngle += 360.0f;
		}
		finalAngle = -finalAngle;
	}

	if (finalAngle > 50.0f)
	{
		finalAngle = 50.0f;
	}
	else if (finalAngle < -50.0f)
	{
		finalAngle = -50.0f;
	}
	
	headsprite->SetAngle(finalAngle);
	gunsprite->SetAngle(finalAngle);
}

// 通常時の更新処理
void Player::updateNormal()
{	
	if (StopState)
	{
		return;
	}

	int weaponID = GetCurrentWeapon();
	CDirectInput* input = GameDevice()->m_pDI;

	if (input->CheckMouse(KD_TRG, DIM_LBUTTON))
	{
			if (shottrriger == false)
			{
				POINT pt;
				GetCursorPos(&pt);
				ScreenToClient(GameDevice()->m_pMain->m_hWnd, &pt);
				VECTOR2 guncenter = gunsprite->GetCenter();

				VECTOR2 mouseWorldPos = {
					static_cast<float>(pt.x) + GameDevice()->Scroll.x,
					static_cast<float>(pt.y) + GameDevice()->Scroll.y
				};

				VECTOR2 offset = {
					-transform.center.x + (gunsprite->GetWidth() * 2) + 2,
					-transform.center.y + (guncenter.y * 4)
				};

				XMMATRIX matScale = XMMatrixScaling(transform.scale.x, transform.scale.y, 1.0f);
				XMMATRIX matRot = XMMatrixRotationZ(XMConvertToRadians(transform.rotation));
				XMMATRIX matTrans = XMMatrixTranslation(transform.position.x, transform.position.y, 0.0f);

				XMMATRIX worldMat = matScale * matRot * matTrans;

				XMVECTOR localVec = XMVectorSet(offset.x, offset.y, 0.0f, 1.0f);
				XMVECTOR worldVec = XMVector4Transform(localVec, worldMat);

				XMStoreFloat3(&shotWorldPos, worldVec);

				dir = mouseWorldPos - VECTOR2(shotWorldPos.x, shotWorldPos.y);
				angleRad = atan2f(dir.y, dir.x);
				angleDeg = angleRad * (180.0f / 3.14159265f);
				float Langle = angleDeg;

				if (Langle < -70.0f)
				{
					if (Langle > -110.0f)
					{
						if (rigthside)
						{
							Langle = -110.0f;
						}
						else
						{
							Langle = -70.0f;
						}
					}
				}
				else if(Langle > 70.0f)
				{
					if (Langle < 130.0f)
					{
						if (rigthside)
						{
							Langle = 130.0f;
						}
						else
						{
							Langle = 70.0f;
						}
					}
				}

				switch (weaponID)
				{
				case Weapon_Shot:
					if (mg > 0)
					{
						weapon->Spawn<WeaponShot>(VECTOR2{ shotWorldPos.x, shotWorldPos.y }, Langle, WeaponBase::ePC);
						mg -= 1;
					}
					break;
				case Weapon_Penetrate:
					if (pnmg > 0)
					{
						weapon->Spawn<WeaponPenetrate>(VECTOR2{ shotWorldPos.x, shotWorldPos.y }, Langle, WeaponBase::ePC);
						pnmg -= 1;
					}
					break;
				case Weapon_Grenade:
					if (grmg > 0)
					{
						weapon->Spawn<WeaponGrenade>(VECTOR2{ shotWorldPos.x, shotWorldPos.y }, Langle, WeaponBase::ePC);
						grmg -= 1;
					}
					break;
				}
				
				waitstrat = std::chrono::steady_clock::now();

				shottrriger = true;
			}
	}
	
	if (input->CheckKey(KD_TRG, DIK_R))
	{
		startTime = std::chrono::steady_clock::now();

		if (!Reloadmode)
		{
			Reloadtimer = 0.0f;
			MoveSpeed = OriginalMoveSpeed / 4;
			Reloadmode = true;
			jumppossibility = false;
		}
	}
	
	if (input->CheckKey(KD_TRG, DIK_Q))
	{
		switchWeapon();
		GetCurrentWeapon();
	}

	if (input->CheckMouse(KD_TRG, DIM_RBUTTON))
	{
		if (hook == nullptr && !IsGrapplingHookActive())
		{
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(GameDevice()->m_pMain->m_hWnd, &pt);
			VECTOR2 guncenter = gunsprite->GetCenter();

			VECTOR2 mouseWorldPos = {
				static_cast<float>(pt.x) + GameDevice()->Scroll.x,
				static_cast<float>(pt.y) + GameDevice()->Scroll.y
			};

			VECTOR2 offset = {
				-transform.center.x + (gunsprite->GetWidth() * 2) + 2,
				-transform.center.y + (guncenter.y * 4)
			};

			XMMATRIX matScale = XMMatrixScaling(transform.scale.x, transform.scale.y, 1.0f);
			XMMATRIX matRot = XMMatrixRotationZ(XMConvertToRadians(transform.rotation));
			XMMATRIX matTrans = XMMatrixTranslation(transform.position.x, transform.position.y, 0.0f);

			XMMATRIX worldMat = matScale * matRot * matTrans;

			XMVECTOR localVec = XMVectorSet(offset.x, offset.y, 0.0f, 1.0f);
			XMVECTOR worldVec = XMVector4Transform(localVec, worldMat);

			XMFLOAT3 shotWorldPos;
			XMStoreFloat3(&shotWorldPos, worldVec);

			dir = mouseWorldPos - VECTOR2(shotWorldPos.x, shotWorldPos.y);
			angleRad = atan2f(dir.y, dir.x);
			angleDeg = angleRad * (180.0f / 3.14159265f);
			float Langle = angleDeg;

			if (Langle < -70.0f)
			{
				if (Langle > -110.0f)
				{
					if (rigthside)
					{
						Langle = -110.0f;
					}
					else
					{
						Langle = -70.0f;
					}
				}
			}
			else if (Langle > 70.0f)
			{
				if (Langle < 130.0f)
				{
					if (rigthside)
					{
						Langle = 130.0f;
					}
					else
					{
						Langle = 70.0f;
					}
				}
			}

			CSpriteImage* hookSpriteImage = weapon->GetImage();

			grapplinghook* newHook = weapon->Spawn<grapplinghook>(VECTOR2{ shotWorldPos.x, shotWorldPos.y }, Langle, WeaponBase::ePC);
			
			if (newHook) {
				CSpriteImage* ropeImage = GameDevice()->GetSpriteImage("C:/C2DAct114/Gunletics/Data/Image/my_ammo.png");
				newHook->SetRopeImage(ropeImage);
				SetGrapplingHook(newHook);
				isgrappling = true;
			}
		}
		else if (hook != nullptr)
		{
			isgrappling = true;
		}
	}
	else
	{
		isgrappling = false;
	}
}

void Player::Reload()
{
	if (Reloadmode) 
	{
		int weaponID = GetCurrentWeapon();
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<float> elapsed = now - startTime;

		Reloadtimer = elapsed.count();

		if (Reloadtimer >= Reloadtime)
		{
			switch (weaponID)
			{
			case Weapon_Shot:
				mg = MaxMg;
				break;
			case Weapon_Penetrate:
				pnmg = MaxPnMg;
				break;
			case Weapon_Grenade:
				grmg = MaxGrMg;
				break;
			}
			MoveSpeed = OriginalMoveSpeed;
			Reloadmode = false;
			jumppossibility = true;
		}
	}
}

void Player::updateDamage()
{
	hp -= beDamaged;	   // 相手からのダメージを計算
	beDamaged = 0;
	if (hp <= 0)
	{
		hp = 0;			   //	
		wait = DeadWaitTime;
		state = stDead;	   // 死亡処理へ
		animator->SetFlash();    // アニメーションをフラッシュ状態にする
		headsprite->Flash(wait);
		gunsprite->Flash(wait);
	}
	else {
		wait = FlashWaitTime;
		state = stFlash;	     // フラッシュ状態にする
		animator->SetFlash();    // アニメーションをフラッシュ状態にする
		headsprite->Flash(wait);
		gunsprite->Flash(wait);
	}
}

void Player::updateDead()
{
	DataCarrier* data = ObjectManager::FindGameObject<DataCarrier>();
	data->Setsecond(second);
	data->Settensecond(tensecond);
	data->Setminute(minute);
	isDead = true;
	if (--wait <= 0)
	{
		if (--num <= 0)	  // PCをひとつ減らす
		{
			SceneManager::ChangeScene("OverScene");
		}
		else {
			
			hp = MaxHp;
			wait = FlashWaitTime;
			state = stFlash;	     // 状態だけフラッシュ状態にする
			animator->ResetFlash();
		}
	}
}
// 歩行中の処理
void Player::updateWalk()
{
	CDirectInput* input = GameDevice()->m_pDI;
	float currentHorizontalSpeed = 0.0f;

	if (input->CheckKey(KD_DAT, DIK_D))
	{
		if (rigthside)
		{
			animator->SetAnimationRange(0, 7, 1, false);
		}
		else
		{
			animator->SetAnimationRange(0, 7, 0, true);
		}
		currentHorizontalSpeed = MoveSpeed;
	}
	else if (input->CheckKey(KD_DAT, DIK_A))
	{
		if (rigthside)
		{
			animator->SetAnimationRange(0, 7, 1, true);
			
		}
		else
		{
			animator->SetAnimationRange(0, 7, 0, false);
		}
		currentHorizontalSpeed = -MoveSpeed;
	}
	else
	{
		if (rigthside)
		{
			animator->SetAnimationRange(0, 1, 1, false);
		}
		else
		{
			animator->SetAnimationRange(0, 1, 0, false);
		}
	}

	velocity.x += (currentHorizontalSpeed - velocity.x) * GroundFriction;

	if (jumppossibility == true && (input->CheckKey(KD_DAT, DIK_SPACE) || input->CheckJoy(KD_DAT, DIJ_UP))) //	↑キー
	{	
		// ジャンプ開始
		atcstate = AtcState::atJump;
		jumpSpeed.x = velocity.x;
		jumpSpeed.y = -JumpSpeed;
		velocity.y = jumpSpeed.y;
		jumpTime = 0;

		if (rigthside)
		{
			animator->SetAnimationRange(8, 1, 1, false);
		}
		else
		{
			animator->SetAnimationRange(8, 1, 0, false);
		}
	}
	else {
		// 自然落下
		if (input->CheckKey(KD_DAT, DIK_DOWN) || input->CheckJoy(KD_DAT, DIJ_DOWN))//↓キー
		{
			velocity.y = MoveSpeed;
			animator->SetDir(Animator::diDown);
		}
		else {
			atcstate = AtcState::atJump;
			jumpSpeed.x = velocity.x;
			jumpSpeed.y = MoveSpeed / 2;
			jumpTime = 0;
			velocity.y = jumpSpeed.y;
		}
	}
}

// ジャンプ中の処理
void Player::updateJump()
{	
	CDirectInput* input = GameDevice()->m_pDI;
	float airControlSpeed = 0.0f;

	if (input->CheckKey(KD_DAT, DIK_D))
	{
		airControlSpeed = MoveSpeed;
	}
	if (input->CheckKey(KD_DAT, DIK_A))
	{
		airControlSpeed = -MoveSpeed;
	}
	if (rigthside)
	{
		animator->SetAnimationRange(8, 1, 1, false);
	}
	else
	{
		animator->SetAnimationRange(8, 1, 0, false);
	}

	jumpTime++;
	velocity.x += (airControlSpeed - velocity.x) * AirControlAcceleration;
}

void Player::updateStop()
{

}

void Player::shotwaittime()
{
	if (shottrriger)
	{
		auto wait = std::chrono::steady_clock::now();
		std::chrono::duration<float> waitdur = wait - waitstrat;

		waittimer = waitdur.count();

		if (waittimer >= waittime)
		{
			shottrriger = false;
		}
	}
}

void Player::HandleSwingInput()
{
	if (!hook) return;

	CDirectInput* input = GameDevice()->m_pDI;

	const float SWING_ACCELERATION_FORCE = 1.0f;
	const float SWING_DECELERATION_FORCE = 1.5f;

	VECTOR2 anchorPoint = hook->Getanchorpoint();
	VECTOR2 playerPos = transform.position;
	VECTOR2 directionToAnchor = anchorPoint - playerPos;
	float currentDistance = CalculateDistance(playerPos, anchorPoint);

	if (currentDistance < 0.001f) return;

	VECTOR2 radialDirection = directionToAnchor;
	radialDirection.x /= currentDistance;
	radialDirection.y /= currentDistance;

	VECTOR2 tangentialDirectionClockwise = { -radialDirection.y, radialDirection.x };
	VECTOR2 tangentialDirectionCounterClockwise = { radialDirection.y, -radialDirection.x };

	float currentTangentialSpeed = velocity.x * tangentialDirectionClockwise.x + velocity.y * tangentialDirectionClockwise.y;

	VECTOR2 currentSwingDirection;
	if (currentTangentialSpeed > 0.01f) {
		currentSwingDirection = tangentialDirectionClockwise;
	}
	else if (currentTangentialSpeed < -0.01f) {
		currentSwingDirection = tangentialDirectionCounterClockwise;
	}
	else {
		currentSwingDirection = VECTOR2(0, 0);
	}

	VECTOR2 calculatedForce = VECTOR2(0, 0);

	if (input->CheckKey(KD_DAT, DIK_A)) {
		float dotWithCurrentSwing = tangentialDirectionCounterClockwise.x * currentSwingDirection.x +
			tangentialDirectionCounterClockwise.y * currentSwingDirection.y;

		if (dotWithCurrentSwing > 0.0f) {
			calculatedForce.x += tangentialDirectionCounterClockwise.x * SWING_ACCELERATION_FORCE;
			calculatedForce.y += tangentialDirectionCounterClockwise.y * SWING_ACCELERATION_FORCE;
		}
		else {
			calculatedForce.x += tangentialDirectionCounterClockwise.x * SWING_DECELERATION_FORCE;
			calculatedForce.y += tangentialDirectionCounterClockwise.y * SWING_DECELERATION_FORCE;
		}
	}
	if (input->CheckKey(KD_DAT, DIK_D)) {
		float dotWithCurrentSwing = tangentialDirectionClockwise.x * currentSwingDirection.x +
			tangentialDirectionClockwise.y * currentSwingDirection.y;

			calculatedForce.x += tangentialDirectionClockwise.x * SWING_ACCELERATION_FORCE;
			calculatedForce.y += tangentialDirectionClockwise.y * SWING_ACCELERATION_FORCE;
		}
		else {
			calculatedForce.x += tangentialDirectionClockwise.x * SWING_DECELERATION_FORCE;
			calculatedForce.y += tangentialDirectionClockwise.y * SWING_DECELERATION_FORCE;
		}
	swing = calculatedForce;
}

float Player::CalculateDistance(const VECTOR2& pos1, const VECTOR2& pos2)
{
	float dx = pos1.x - pos2.x;
	float dy = pos1.y - pos2.y;
	return sqrtf(dx * dx + dy * dy);
}
