#include "GameMain.h"
#include "Sprite.h"
#include "Collider.h"
#include "Player.h"
#include "EnemyBase.h"
#include "EffectManager.h"
#include "ObjectManager.h"
#include "AudioManager.h"
#include "Map.h"
#include "grapplinghook.h"
#include <math.h>

namespace {
	const int MoveSpeed = 40;
	const int MaxAtc = 100;
	const VECTOR4 SrcPattern = VECTOR4(72, 7, 32, 18);
	const VECTOR4 SrcRope = VECTOR4(72, 0, 33, 4);
	const int MAXMG = 10;
	const float MaxRange = 500.0f;
	const float GrappleSpeed = 15.0f;
	const float PI = 3.14159265f;
	static const int ROPE_SEGMENTS = 32;
	const float MAX_SAG_DISTANCE = 100.0f;
	const float MIN_SAG_DISTANCE = 300.0f;
	const float BASE_MAX_SAG = 40.0f;
	const float ANCHOR_SAG_REDUCTION_FACTOR = 0.5f;
	const float CORRECTION_STRENGTH = 0.6f;
	const float RADIAL_DAMPING = 0.1f;
	const float SWING_DAMPING_FACTOR = 0.99f;
	const float PULL_SPEED = 12.0f;
	const float ROPE_SHORTEN_SPEED = 2.0f;
	const float MIN_ROPE_LENGTH = 10.0f;
};

grapplinghook::grapplinghook(CSpriteImage* image) : WeaponBase(), ropeimage(nullptr)
{
	// スプライトとアニメーター,コリジョンの初期化
	CreateSprite(image, SrcPattern);
	transform.center.x = sprite->GetSrcWidth() / 2;
	transform.center.y = sprite->GetSrcHeight() / 2;

	col = new Collider(this);
	animator = new Animator(this);
	objMap = nullptr;
	state = State::stNormal;
	atc = MaxAtc;
	isAnchored = false;
	ishookonline = true;
}

grapplinghook::~grapplinghook()
{
	SAFE_DELETE(col);
	SAFE_DELETE(animator);
	SAFE_DELETE(sprite);
}

void grapplinghook::Start()
{
	objMap = ObjectManager::FindGameObject<Map>();
	PL = ObjectManager::FindGameObject<Player>();

	float rad = transform.rotation * Deg2Rad;

	velocity.x = cosf(rad) * MoveSpeed;
	velocity.y = sinf(rad) * MoveSpeed;

	if (PL) {
		startPos = PL->GetshotPos();
	}
}

void grapplinghook::Update()
{
	playerPos = PL->GetshotPos();

	if (!WeaponupdateEnabled)
	{
		return;
	}

	switch (state) {
	case State::stNormal:
		updateNormal();
		break;
	case State::stAnchored:
		updateAnchored();
		break;
	case State::stRewinding:
		updateRewinding();
		break;
	}

	animator->Update();

	if(!isAnchored)
	{
		transform.position += velocity;
	}
}

void grapplinghook::Draw()
{
    DrawRope();
	Object2D::Draw();
}

void grapplinghook::DrawRope()
{
	if(!PL || !ropeimage) return;

	VECTOR2 playerPos = PL->GetshotPos(); 
	VECTOR2 hookPos = transform.position; 

	float hookRotationRad = transform.rotation * Deg2Rad;

	float offsetX = -cosf(hookRotationRad) * (SrcPattern.z / 2.0f); 
	float offsetY = -sinf(hookRotationRad) * (SrcPattern.z / 2.0f); 

	VECTOR2 actualHookConnectPoint = { hookPos.x + offsetX, hookPos.y + offsetY };


	float totalDistance = CalculateDistance(playerPos, actualHookConnectPoint);
	VECTOR2 direction = actualHookConnectPoint - playerPos;

	if (totalDistance < 1.0f) return;

	direction.x /= totalDistance;
	direction.y /= totalDistance;

	float segmentLength = totalDistance / ROPE_SEGMENTS;

	VECTOR2 scroll = GameDevice()->Scroll;

	for (int i = 0; i < ROPE_SEGMENTS; i++) 
	{
		float t = (float)i / ROPE_SEGMENTS;
		VECTOR2 currentPos;
		currentPos.x = playerPos.x + direction.x * totalDistance * t;
		currentPos.y = playerPos.y + direction.y * totalDistance * t;

		float sag = CalculateRopeSag(t, totalDistance);
		currentPos.y += sag;

		float angle = 0.0f;
		if (i < ROPE_SEGMENTS - 1) {
			float nextT = (float)(i + 1) / ROPE_SEGMENTS;
			VECTOR2 nextPos;
			nextPos.x = playerPos.x + direction.x * totalDistance * nextT;
			nextPos.y = playerPos.y + direction.y * totalDistance * nextT + CalculateRopeSag(nextT, totalDistance);

			VECTOR2 segmentDir = nextPos - currentPos;
			float segmentDist = CalculateDistance(segmentDir, VECTOR2{ 0,0 });
			if (segmentDist > 0.001f) { 
				angle = atan2f(segmentDir.y, segmentDir.x) * Rad2Deg;
			}
			else {
				angle = transform.rotation; 
			}
		}
		else {
			angle = transform.rotation;
		}

		VECTOR2 drawPos = currentPos - scroll;

		DrawRopeSegment(drawPos, angle, segmentLength);
	}

}

void grapplinghook::updateNormal()
{
	if (!PL)
	{
		Destroy();
	}

	float distancefromStart = CalculateDistance(startPos, transform.position);

	if (distancefromStart > MaxRange)
	{
		state = State::stRewinding;
	}

	if (HitCheck()) 
	{
		state = State::stRewinding;
	}

	MapLine* pHitMapLine = nullptr;

	if (objMap && objMap->isCollisionMoveMap(this, velocity, pHitMapLine))
	{
		SetAnchorPoint(transform.position);
	}
}

float grapplinghook::CalculateDistance(const VECTOR2& pos1, const VECTOR2& pos2)
{
	float dx = pos1.x - pos2.x;
	float dy = pos1.y - pos2.y;
	return sqrtf(dx * dx + dy * dy);
}

void grapplinghook::DrawRopeSegment(const VECTOR2& pos, float angle, float length)
{
	if (!ropeimage) return;

	float originalRopeWidth = SrcRope.z;  
	float originalRopeHeight = SrcRope.w; 

	float scaleX = length / SrcRope.z; 
	float scaleY = 1.0f; 

	VECTOR2 segmentCenter = VECTOR2(0.0f, originalRopeHeight / 2.0f);

	CSprite tempSprite(ropeimage, SrcRope.x, SrcRope.y, SrcRope.z, SrcRope.w);


	MATRIX4X4 mTranslateOrigin = XMMatrixTranslation(-segmentCenter.x, -segmentCenter.y, 0.0f);
	MATRIX4X4 mScale = XMMatrixScaling(scaleX, scaleY, 1.0f);
	MATRIX4X4 mRotationZ = XMMatrixRotationZ(angle * Deg2Rad);

	MATRIX4X4 mWorld = mTranslateOrigin * mScale * mRotationZ * XMMatrixTranslation(pos.x, pos.y, 0.0f);

	tempSprite.Draw(mWorld);
}

void grapplinghook::SetAnchorPoint(const VECTOR2& point)
{
	anchorPoint = point;
	isAnchored = true;
	state = State::stAnchored;
	velocity = VECTOR2(0, 0);

	if (PL)
	{
		ropeLength = CalculateDistance(PL->GetshotPos(), anchorPoint);
	}

	transform.position = anchorPoint;
}

void grapplinghook::updateAnchored()
{
	CDirectInput* input = GameDevice()->m_pDI;

	if (!PL)
	{
		Destroy();
	}

	bool wasGrappling = isGrappling;

	if (input->CheckMouse(KD_DAT, DIM_RBUTTON))
	{
		PL->SetGravity(0.6f);
		PL->SetCame(true);
		isGrappling = true;
	}
	else
	{
		PL->DefalteGravity();
		PL->SetCame(false);
		isGrappling = false;
	}

	if (input->CheckKey(KD_TRG, DIK_X))
	{
		isGrappling = false;
		isAnchored = false;
		state = State::stRewinding;
	}

	updateGrappling();
}

void grapplinghook::updateGrappling()
{
	if (!PL || !isAnchored) return;
	VECTOR2 playerPos = PL->GetshotPos();
	VECTOR2 currentVelocity = PL->velocity;

	VECTOR2 directionToAnchor = anchorPoint - playerPos;
	float currentDistance = CalculateDistance(playerPos, anchorPoint);

	if (ropeLength <= 0.001f || currentDistance < 0.001f) {
		PL->velocity = VECTOR2(0, 0);
		PL->FalseGrappling();
		Destroy();
		return;
	}

	VECTOR2 unitDirToAnchor = directionToAnchor / currentDistance;

	const float GRAVITY_ACCELERATION = PL->GetBasegravity();
	VECTOR2 gravityVector = { 0.0f, GRAVITY_ACCELERATION };
	currentVelocity += gravityVector;

	if (isGrappling)
	{
		ropeLength -= PULL_SPEED;

		if (currentDistance < MIN_ROPE_LENGTH) {
			Destroy();
		}
		currentVelocity = unitDirToAnchor * PULL_SPEED;
	}
	else if(PL->GetOnGround() == false)
	{
		VECTOR2 idealPlayerPosOnCircle = anchorPoint - unitDirToAnchor * ropeLength;
		VECTOR2 positionCorrection = idealPlayerPosOnCircle - playerPos;

		currentVelocity += positionCorrection * CORRECTION_STRENGTH;

		float dotProduct = currentVelocity.x * unitDirToAnchor.x + currentVelocity.y * unitDirToAnchor.y;
		VECTOR2 radialVelocity = unitDirToAnchor * dotProduct;
		VECTOR2 tangentialVelocity = currentVelocity - radialVelocity;

		radialVelocity *= RADIAL_DAMPING;

		tangentialVelocity *= SWING_DAMPING_FACTOR;

		currentVelocity = tangentialVelocity + radialVelocity + PL->GetSwingInputForce();
	}

	PL->velocity = currentVelocity;
	PL->ResetSwingInputForce();
}

void grapplinghook::updateRewinding()
{
	if (!PL)
	{
		Destroy();
	}

	VECTOR2 playercurrentPos = PL->GetshotPos();

	VECTOR2 directionToPlayer = playercurrentPos - transform.position;
	float distanceToplayer = CalculateDistance(playercurrentPos, transform.position);

	if (distanceToplayer < GrappleSpeed)
	{
		Destroy();
	}

	if (distanceToplayer > 30.0f)
	{
		directionToPlayer.x /= distanceToplayer;
		directionToPlayer.y /= distanceToplayer;

		velocity.x = directionToPlayer.x * (MoveSpeed * 1.5f);
		velocity.y = directionToPlayer.y * (MoveSpeed * 1.5f);
	}
	else
	{
		Destroy();
	}
}

void grapplinghook::Destroy()
{
	ishookonline = false;
	isAnchored = false;
	PL->DefalteGravity();
	PL->FalseGrappling();
	PL->SetCame(false);
	DestroyMe();
}

float grapplinghook::CalculateRopeSag(float t, float totalDistance)
{
	float sagFactor = 0.0f;

	if (totalDistance <= MAX_SAG_DISTANCE) {
		sagFactor = 1.0f;
	}
	else if (totalDistance >= MIN_SAG_DISTANCE) {
		sagFactor = 0.0f;
	}
	else {
		sagFactor = 1.0f - ((totalDistance - MAX_SAG_DISTANCE) / (MIN_SAG_DISTANCE - MAX_SAG_DISTANCE));
		sagFactor = max(0.0f, min(1.0f, sagFactor)); // 念のためクランプ
	}

	float currentSagAmount = BASE_MAX_SAG * sagFactor;

	return currentSagAmount * 4.0f * t * (1.0f - t);
}
