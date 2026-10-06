#include "EnemyManager.h"
#include "EnemyBase.h"
#include "Player.h"
#include "DataCarrier.h"


EnemyBase::EnemyBase()
{
	SetTag("Enemy");
	state = stNormal;
	atcstate = atWalk;
	speedY = 0;
	hitPoint = 0;
	flashTimer= 0;
	idleTimer= 0;
}

EnemyBase::~EnemyBase()
{
	navigationMap.clear();
	navigationMap.shrink_to_fit();
}

void EnemyBase::MakeNavigationMap(std::vector<VECTOR3> nvIn)
{
	navigationMap.clear();
	navigationMap.shrink_to_fit();
	for (const VECTOR3& nv : nvIn)
	{
		navigationMap.emplace_back(nv);
	}
	if (navigationMap.size() > 0)
	{
		transform.position = navigationMap[0];
	}
}

void EnemyBase::AddDamage(float damage, VECTOR3 pPos)
{
	if (state != stNormal )	  return;

	hitPoint -= damage;
	if (hitPoint > 0) {
		VECTOR3 push = transform.position - pPos;
		push.y = 0;
		push = XMVector3Normalize(push) * 0.4f;
		transform.position += push;
		transform.rotation.y = atan2f(-push.x, -push.z);
		state = stDamage;
	}
	else {
		state = stDead;
	}
}


bool EnemyBase::MoveToTarget(VECTOR3 target, float speed, float rotSpeed)
{
	const float NearLimit = 0.5f;

	VECTOR3 toTarget = target - transform.position;
	if (magnitude(toTarget) <= NearLimit) return true;

	MATRIX4X4 myRot = XMMatrixRotationY(transform.rotation.y);
	VECTOR3 front = VECTOR3(0, 0, 1) * myRot;

	float forward = speed;

	if (mesh->GetRootAnimType(animator->PlayingID()) != eRootAnimNone)
	{
		forward = GetPositionVector(mesh->GetRootAnimUpMatrices(animator)).z;
	}
	transform.position += front * forward * 60 * SceneManager::DeltaTime();
	VECTOR3 right = VECTOR3(1, 0, 0) * myRot;

	VECTOR3 toTarget1 = XMVector3Normalize(toTarget);
	float ip = Dot(front, toTarget1);
	float rSpeed = rotSpeed * DegToRad * 60 * SceneManager::DeltaTime();
	if (ip >= cosf(rSpeed)) {
		transform.rotation.y = atan2f(toTarget.x, toTarget.z);
	}
	else if (Dot(right, toTarget) > 0) {
		transform.rotation.y += rSpeed;
	}
	else {
		transform.rotation.y -= rSpeed;
	}

	return false;
}

bool EnemyBase::CheckReach(Object3D* player, float angle, float ReachDistLimit)
{
	VECTOR3 toPlayer = player->Position() - transform.position;

	if (toPlayer.Length() < ReachDistLimit / 3)  return true;

	if (toPlayer.Length() < ReachDistLimit) {
		VECTOR3 front = VECTOR3(0, 0, 1) * XMMatrixRotationY(transform.rotation.y);
		VECTOR3 toPlayer1 = XMVector3Normalize(toPlayer);
		float ip = Dot(front, toPlayer1);
		if (ip >= cosf(angle * DegToRad)) {
			return true;
		}
	}
	return false;
}


SphereCollider EnemyBase::Collider()
{
	SphereCollider col;
	col.radius = 1.5f;
	col.center = transform.position + VECTOR3( 0, col.radius, 0 );
	return col;
}

