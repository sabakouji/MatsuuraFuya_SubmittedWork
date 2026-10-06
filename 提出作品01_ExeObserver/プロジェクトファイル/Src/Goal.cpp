#include "Goal.h"
#include "Executor.h"
#include "SceneManager.h"
#include <iostream>
#include <cmath>

//-----------------------------------------------------------------------------
// 定数
namespace
{
	// デフォルトのトリガー半径
	const float DefaultTriggerRadius = 1.5f;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 初期化
Goal::Goal()
	: m_triggerRadius(DefaultTriggerRadius),
	m_triggered(false)
{
	mesh = new CFbxMesh();
	mesh->Load("Data/Map/MapItem/Shade.mesh");

	meshCol = new MeshCollider();
	meshCol->MakeFromMesh(mesh);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 終了
Goal::~Goal()
{
	SAFE_DELETE(mesh);
	SAFE_DELETE(meshCol);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 更新
void Goal::Update()
{
	// すでにトリガーされている場合は何もしない
	if (m_triggered) return;

	// Executorオブジェクトを探す
	Executor* executor = ObjectManager::FindGameObject<Executor>();
	if (!executor) return;

	// 自分とExecutorの位置を取得
	VECTOR3 myPos = transform.position;
	VECTOR3 exePos = executor->Position();

	// XZ平面での距離を計算
	float dx = exePos.x - myPos.x;
	float dz = exePos.z - myPos.z;
	float distance = std::sqrt(dx * dx + dz * dz);

	// トリガー半径内に入ったらクリアシーンへ
	if (distance <= m_triggerRadius)
	{
		m_triggered = true;
		SceneManager::ChangeScene("ClearScene");
	}
}

//-----------------------------------------------------------------------------
// 描画
void Goal::Draw()
{
	Object3D::Draw();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ゴールの情報をテキストから読み込んで設定する
void Goal::MakeGoal(TextReader* txt, int n)
{
	// 位置と回転を読み込む
	VECTOR3 pos, rot;

	// 位置は必須
	pos.x = txt->GetFloat(n, 1);
	pos.y = txt->GetFloat(n, 2);
	pos.z = txt->GetFloat(n, 3);
	transform.position = pos;

	// 回転は任意
	if (txt->GetColumns(n) > 4)
	{
		rot.x = 0;
		rot.y = txt->GetFloat(n, 4);
		rot.z = 0;
		transform.rotation = rot * DegToRad;
	}

	// トリガー半径は任意
	if (txt->GetColumns(n) > 5)
	{
		m_triggerRadius = txt->GetFloat(n, 5);
	}

	// タグは任意
	if (txt->GetColumns(n) > 6)
	{
		std::string tag = txt->GetString(n, 6);
		SetTag(tag);
	}
}

//-----------------------------------------------------------------------------
// ゴールの情報を直接指定して設定する
void Goal::SetupGoal(const VECTOR3& position, float rotationY, float triggerRadius, const std::string& tag)
{
	// 位置と回転を設定
	transform.position = position;
	// Y軸回転のみを設定
	transform.rotation.y = rotationY * DegToRad;
	// トリガー半径とタグを設定
	m_triggerRadius = triggerRadius;
	// タグを設定
	SetTag(tag);
}

//-----------------------------------------------------------------------------
// ゴールのコライダーを返す
SphereCollider Goal::Collider()
{
	// ゴールのコライダーは、ゴールの位置を中心とし、トリガー半径を半径とする球体とする
	SphereCollider sphere;
	sphere.center = transform.position + VECTOR3(0, sphere.radius, 0);
	sphere.radius = m_triggerRadius;
	return sphere;
}
//-----------------------------------------------------------------------------