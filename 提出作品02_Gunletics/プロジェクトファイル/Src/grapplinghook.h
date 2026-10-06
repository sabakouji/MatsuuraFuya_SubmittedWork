#pragma once
#include "Object2D.h"
#include "Animator.h"
#include "Collider.h"
#include "WeaponManager.h"
#include "Map.h"

/// <summary>
/// 武器ショットのクラス
/// </summary>
class Player;
class grapplinghook : public WeaponBase
{
public:
	grapplinghook(CSpriteImage* image);
	~grapplinghook();
	void Start() override;
	void Update() override;
	void Draw() override;

	void SetRopeImage(CSpriteImage* ropeImg) { ropeimage = ropeImg; }
	VECTOR2 Getanchorpoint() { return anchorPoint; };
	bool IsAnchored() { return isAnchored; };
	bool IsGrappling() { return isGrappling; };
	bool IsOnline() { return ishookonline; };
	void chageRewind() { state = State::stRewinding; };

	void Destroy();

	CSpriteImage* m_pImage;
	DWORD m_ofX;
private:
	Map* objMap;
	Player* PL;
	CSpriteImage* ropeimg;
	CSpriteImage* ropeimage;
	VECTOR2 velocity;
	VECTOR2 normal;
	VECTOR2 playerPos;
	VECTOR2 anchorPoint;
	float ropeLength;
	VECTOR2 startPos;
	bool isAnchored;
	bool isGrappling;
	bool ishookonline;

	void DrawRope();
	void updateNormal();
	float CalculateDistance(const VECTOR2& pos1, const VECTOR2& pos2);
	float CalculateRopeSag(float t, float totalDistance);
	void DrawRopeSegment(const VECTOR2& pos, float angle, float length);
	void SetAnchorPoint(const VECTOR2& point);
	void updateAnchored();
	void updateGrappling();
	void updateRewinding();

	
};