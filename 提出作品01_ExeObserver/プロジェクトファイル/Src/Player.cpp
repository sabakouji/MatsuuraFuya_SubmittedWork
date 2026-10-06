#include "Player.h"
#include "../Libs/Imgui/imgui.h"
#include "EnemyManager.h"
#include "MapManager.h"
#include "WeaponManager.h"

namespace {
	const float Gravity = 0.025f; // 重力加速度(正の値)
	const float JumpPower = 0.5f;  // ジャンプの初速
	const float RotationSpeed = 3; // 回転速度(度)
	const float MoveSpeed = 0.1f;  // 移動スピード
	const int   MaxNumber = 3;	   // プレイヤーの数
	const int   MaxHitPoint = 1000;
	const float MaxFlashTime = 5;
	const float MaxDeadTime = 100;
};

Player::Player()
{
	animator = new Animator(); // インスタンスを作成

	// Maria
	mesh = new CFbxMesh();
	mesh->Load("Data/Char/Maria/Maria.mesh");
	mesh->LoadAnimation(aIdle, "Data/Char/Maria/Maria_Idle.anmx", true);
	mesh->LoadAnimation(aRun, "Data/Char/Maria/Maria_Running.anmx", true);
	mesh->LoadAnimation(aDead, "Data/Char/Maria/Maria_Dying.anmx", false);
	mesh->LoadAnimation(aAttack1, "Data/Char/Maria/Maria_Slash.anmx", false);
	mesh->LoadAnimation(aAttack2, "Data/Char/Maria/Maria_Slash.anmx", false);
	mesh->LoadAnimation(aAttack3, "Data/Char/Maria/Maria_Slash.anmx", false);

	swordObj = ObjectManager::FindGameObject<WeaponManager>()->Spawn<WeaponSword>(WeaponBase::ePC);	  // 剣の発生　　
	swordObj->SetWeaponSword("Sword", 0, 43, VECTOR3(0, 0, 0), VECTOR3(0.0f, 0.01f, -0.02f), VECTOR3(0.0f, 0.0f, -90.0f));  // 剣のメッシュ、手首の位置(0,43は手首) ,長さはメッシュで指定、アジャストの位置と角度
	swordObj->SetParent(this);
	swordObj->SetToonEnabled(true);                                    // 剣もトゥーンで描画する
	swordObj->SetOutline(true, VECTOR4(0.0f, 0.0f, 0.0f, 1.0f), 1.5f);

	gunObj = ObjectManager::FindGameObject<WeaponManager>()->Spawn<WeaponGun>(WeaponBase::ePC);	  // 銃の発生
	//gunObj->SetWeaponGun("Pistol", 0, 43, VECTOR3(0,0,0), VECTOR3(0.0f, 0.0f, 0.0f),VECTOR3(180.0f, 0.0f, 90.0f));  // 銃のメッシュ　　手首の位置(0,43は手首) ,銃口位置はメッシュで指定、アジャストの位置と角度
	gunObj->SetWeaponGun("", 0, 43, VECTOR3(0, 0, 0));   // 銃メッシュなし、手首の位置(0,43は手首) ,銃口位置は(0,0,0)
	gunObj->SetParent(this);
	gunObj->SetToonEnabled(true);                                      // 銃もトゥーンで描画する
	gunObj->SetOutline(true, VECTOR4(0.0f, 0.0f, 0.0f, 1.0f), 1.5f);


	animator->SetModel(mesh); // このモデルでアニメーションする
	animator->Play(aRun);
	animator->SetPlaySpeed(1.0f);

	// トゥーンシェーディングと輪郭線を有効にする
	SetToonEnabled(true);
	SetOutline(true, VECTOR4(0.0f, 0.0f, 0.0f, 1.0f), 2.0f);

	meshCol = new MeshCollider();
	meshCol->MakeFromMesh(mesh, animator);

	transform.position = VECTOR3(0, 0, 0);
	transform.rotation = VECTOR3(0, 0, 0);
	state = stNormal;
	atcstate = atIdle;
	speedY = 0;
	number = MaxNumber;
	hitPoint = MaxHitPoint;
	flashTimer = 0;

	velocity = VECTOR3(0, 0, 0);

	SetDrawOrder(-10);

	swordObj->SetActive(false);
}

Player::~Player()
{
	SAFE_DELETE(mesh);
	SAFE_DELETE(meshCol);
}

float Player::HpdivMax()
{
	return (float)hitPoint / MaxHitPoint;
}

void Player::SetMaxHp()
{
	hitPoint = MaxHitPoint;
}

SphereCollider Player::Collider()
{
	// 少し小さめのバウンディングボールとする
	SphereCollider col;
	col.radius = 0.5f;
	col.center = transform.position + VECTOR3(0, 0.7f, 0);
	return col;
}

void Player::Update()
{
	VECTOR3 positionOld = transform.position;
	velocity = VECTOR3(0, 0, 0);

	switch (state) {
	case stFlash:
		flashTimer -= 60 * SceneManager::DeltaTime();
		if (flashTimer <= 0) state = stNormal;
		updateNormal();
		break;
	case stNormal:
		updateNormal();
		break;
	case stDamage:
		updateDamage();
		break;
	case stDead:
		updateDead();
	}


	ImGui::Begin("Sample");
	ImGui::InputInt("State", (int*)(&state));
	ImGui::InputFloat("SP", &speedY);
	ImGui::End();

	// マップとの接触判定と自然落下処理
	transform.position.y += speedY;		 // 自然落下速度を加える
	speedY -= Gravity * 60 * SceneManager::DeltaTime();
	MapManager* mm = ObjectManager::FindGameObject<MapManager>();
	if (mm->IsCollisionMoveGravity(positionOld, transform.position) != clFall)
	{
		speedY = 0;
	}

	animator->Update(); // 毎フレーム、Updateを呼ぶ		 // -- 2024.9.5
}

void Player::updateNormal()
{
	switch (atcstate) {
	case atIdle:
		updateNormalWalk();
		break;
	case atWalk:
		updateNormalWalk();
		break;
	case atAttack:
		updateNormalAttack();
		break;
	}
}

void Player::updateNormalWalk()
{
	if (GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_W) || GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_UP)) {
		// 前進
		VECTOR3 forward;
		// ルートボーンアニメーションを行うかどうかルートアニメーションタイプを確認する
		if (mesh->GetRootAnimType(animator->PlayingID()) == eRootAnimNone)
		{
			forward = VECTOR3(0, 0, MoveSpeed * 60 * SceneManager::DeltaTime()); // 回転してない時の移動量 // -- 2024.9.5
		}
		else {
			// ルートボーンアニメーションでの前進移動値
			forward = GetPositionVector(mesh->GetRootAnimUpMatrices(animator)) * 60 * SceneManager::DeltaTime();
		}
		MATRIX4X4 rotY = XMMatrixRotationY(transform.rotation.y); // Yの回転行列
		velocity += forward * rotY; // キャラの向いてる方への移動量
		transform.position += velocity;
		if (atcstate == atAttack)
		{
			animator->MergePlay(aAttack1);
		}
		else {
			animator->MergePlay(aRun);
		}
	}
	else if (GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_S) || GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_DOWN)) {
		// 後退
		VECTOR3 forward = VECTOR3(0, 0, MoveSpeed * 60 * SceneManager::DeltaTime()); // 回転してない時の移動量 // -- 2024.9.5
		MATRIX4X4 rotY = XMMatrixRotationY(transform.rotation.y); // Yの回転行列
		velocity -= forward * rotY; // キャラの向いてる方への移動量
		if (atcstate == atAttack)
		{
			animator->MergePlay(aAttack1);
		}
		else {
			animator->MergePlay(aRun);
		}
	}
	else {
		if (atcstate == atAttack)
		{
			animator->MergePlay(aAttack1);
		}
		else {
			animator->MergePlay(aIdle);
		}
	}
	if (GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_A) || GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_LEFT)) {
		transform.rotation.y -= RotationSpeed * DegToRad * 60 * SceneManager::DeltaTime();	   // -- 2024.9.5
	}
	if (GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_D) || GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_RIGHT)) {
		transform.rotation.y += RotationSpeed * DegToRad * 60 * SceneManager::DeltaTime();	  // -- 2024.9.5
	}
	if (speedY == 0 && GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_SPACE)) {	  // ジャンプボタン
		speedY = JumpPower;
	}

	if (GameDevice()->m_pDI->CheckKey(KD_DAT, DIK_X)) { // ショット攻撃ボタン
		gunObj->ShotLaser(WeaponBase::ePC);
	}

	if (atcstate != atAttack && GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_Z)) { // 剣攻撃ボタン
		animator->MergePlay(aAttack1);
		atcstate = atAttack;
		swordObj->SetActive(true);
	}

	if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_C)) { // 火の玉攻撃ボタン
		VECTOR3 startIn = transform.position + VECTOR3(0, 1.0f, 0);
		VECTOR3 targetIn = XMVector3TransformCoord(VECTOR3(0, 1.0f, 30), transform.matrix());
		ObjectManager::FindGameObject<WeaponManager>()->SpawnMany<WeaponFireBall2>(startIn, targetIn, WeaponBase::ePC);
	}

	transform.position += velocity;	   // 加速度を加える
}


void Player::updateNormalAttack()
{
	if (animator->Finished()) {
		atcstate = atIdle;
		swordObj->SetActive(false);
	}
}

void Player::updateDamage()
{
	state = stFlash;
	flashTimer = MaxFlashTime;
}

void Player::updateDead()
{
	if (animator->Finished())
	{
		if (--number <= 0) // ＰＣを一人減らす
		{
			// ＰＣが全て死亡したらゲームオーバー画面へ
			SceneManager::ChangeScene("OverScene");
		}
		else {
			// 復帰する
			hitPoint = MaxHitPoint;
			state = stFlash;
			flashTimer = MaxDeadTime;
		}
	}
}

void Player::AddDamage(float damage, VECTOR3 pPos)
{
	if (state != stNormal)	  return;  	  // 平常状態以外は当たり判定はなし（無敵状態）

	hitPoint -= damage;
	if (hitPoint > 0) {
		// まだＨＰが残っているとき
		VECTOR3 push = transform.position - pPos;	// 飛ぶ方向のベクトルを作る
		push.y = 0;
		push = XMVector3Normalize(push) * 0.4f;	// そのベクトルの長さを移動数だけ加える
		transform.position += push;	// transform.positionに移動ベクトルを加える
		transform.rotation.y = atan2f(-push.x, -push.z);   // 移動後ろ方向を向く
		state = stDamage; 	// ダメージ状態にする
	}
	else {
		// ＨＰが０になったとき
		animator->Play(aDead);
		state = stDead;	 	// 死亡状態にする
	}
}

void Player::Draw()
{
	Object3D::Draw(); // 継承元の描画関数を呼ぶ
}