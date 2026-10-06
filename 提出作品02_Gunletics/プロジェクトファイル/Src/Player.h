#pragma once
#include "Object2D.h"
#include "Animator.h"
#include "WeaponManager.h"
#include "ItemManager.h"
#include "Map.h"
#include "DInput.h"
#include "RenderComponent.h"
#include "grapplinghook.h"
#include <chrono>

#define Weapon_Shot 1
#define Weapon_Penetrate 2
#define Weapon_Grenade 3

/// <summary>
/// プレイヤークラス
/// </summary>
class ItemManager;
class EnemyManager;
class EnemyBase;
class Player : public Object2D
{
public:
	Player(CSpriteImage* image);
	~Player();
	void Start() override;
	void Update() override;
	void Draw() override;
	void SetBeDamaged(int damaged);

	bool IsNormal() { return state == stNormal; }
	int  Atc() { return atc; }
	int  Num() { return num; }
	int  Hp() { return hp; }
	int  Mp() { return mg; }
	int MaxHP();
	int Getmg();
	int Getsecond() { return second; };
	int GetTenSecond() { return tensecond; };
	int Getminute() { return minute; };
	void SetStop(bool setStop) { stopState = setStop; };
	void SetHpforMax();
	float HpdivMax();
	float MpdivMax();
	VECTOR2 velocity;
	void SetDir(Animator::Dir);
	bool StopState;

	float shotangle;
	float angleDeg;
	VECTOR2 mousePosWorld;

	float finalAngle;
	bool isDead;
	
	void AddWeapon(int weponID);
	bool HasWeapon(int weponID)const;
	void DefalteGravity();
	void switchWeapon();
	int GetCurrentWeapon() const;
	
	VECTOR2 GetPlayerPos()const;

	VECTOR2 Getposition() { return position; };
	void SetPosition(VECTOR2 setPos) { transform.position = setPos; };
	void SetGravity(float scale) { gravity = gravity * scale; };
	
	void FalseGrappling();
	VECTOR2 GetshotPos() { return VECTOR2(shotWorldPos.x, shotWorldPos.y); };
	void SetGrapplingPos(VECTOR2 setvelocity) { transform.position += setvelocity; };
	float Getgravity() { return gravity; };
	float GetBasegravity();
	void SetGrapplingHook(grapplinghook* sethook);
	VECTOR2 GetSwingInputForce() const { return swing; }
	void ResetSwingInputForce() { swing = VECTOR2(0, 0); }
	bool GetOnGround() { return onGround; };
	void SetCame(bool setcame) { iscame = setcame; };

private:
	enum State {
		stNormal = 0,
		stDamage,
		stDead,
		stFlash,
		stStop
	};
	enum AtcState {
		atStanby = 0,
		atChatch,
		atAttack,
		atWalk,
		atJump,
		atRand
	};
	State state;
	AtcState atcstate;

	Map* objMap;
	WeaponManager* weapon;
	Animator* animator;
	EnemyManager* emanager;
	ItemManager* iManager;
	grapplinghook* hook;

	float jumpTime;
	VECTOR2 jumpSpeed;

	int MoveSpeed;
	int wait;
	int shotWait;
	int num;
	int hp;
	int mg;
	int grmg;
	int pnmg;
	int mgsize;
	int atc;
	int beDamaged;
	int maxhp;
	float gravity;
	float frame;
	int second;
	int tensecond;
	int minute;
	POINT point;
	VECTOR2 offset;
	XMFLOAT3 shotWorldPos;
	VECTOR2 scroll;
	VECTOR2 dir;
	XMMATRIX matScale;
	XMMATRIX matRot;
	XMMATRIX matTrans;
	XMMATRIX worldMat;
	XMVECTOR localVec;
	XMVECTOR worldVec;

	CDirectInput* m_pDI;

	RenderComponent* headsprite;
	RenderComponent* gunsprite;
	bool isflash;
	VECTOR2 headoffset;
	VECTOR2 gunoffset;
	VECTOR2 headoffsetpos;
	VECTOR2 gunoffsetpos;
	VECTOR2 headrigthoffsetpos;
	VECTOR2 gunrigthoffsetpos;
	VECTOR2 pivotPos;
	VECTOR2 position;
	VECTOR2 swing;

	float havepoint;
	float dx;
	float dy;
	float length;
	float Reloadtimer;
	float deltatime;
	float retrriger;
	float waittimer;
	float angleRad;
	float Langle;
	bool shottrriger;
	bool Reloadmode;
	bool jumppossibility;
	bool rigthside;
	bool jumpstate;
	bool stopState;
	bool isgrappling;
	bool isokgrappling;
	bool onGround;
	bool iscame;

	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::time_point waitstrat;

	std::vector<EnemyBase*> enemys;

	std::vector<int> weaponInventory;
	int currentweaponIndex = 0;

	void updateNormal();
	void updateDamage();
	void updateDead();
	void updateWalk();
	void updateJump();
	void updateMouse();
	void Reload();
	void updateStop();
	void shotwaittime();
	void HandleSwingInput();
	float CalculateDistance(const VECTOR2& pos1, const VECTOR2& pos2);
	bool IsGrapplingHookActive() const { return (hook != nullptr && hook->IsAnchored()); }
};