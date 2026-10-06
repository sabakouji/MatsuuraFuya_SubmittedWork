#include "MapManager.h"

namespace {
	const float MAP_SKYROTSPEED = 0.005f;
};

MapSky::MapSky()
{
	ObjectManager::SetDrawOrder(this, 10000);   // 一番奥に描画する

	mesh = new CFbxMesh();
}

MapSky::~MapSky()
{
	SAFE_DELETE(mesh);
}

void MapSky::MakeSkyMap(TextReader* txt, int n)
{
	std::string meshName = txt->GetString(n, 1);
	mesh->Load(meshName.c_str());
}
void MapSky::Update()
{
	// 空の更新　　カメラを中心とした位置に置き、Ｙ軸回転させる
	transform.position = GameDevice()->m_vEyePt;
	transform.rotation.y += MAP_SKYROTSPEED * DegToRad * 60 * SceneManager::DeltaTime();	   // ラジアン角にする
	if (transform.rotation.y >= 2 * XM_PI) transform.rotation.y -= 2 * XM_PI;
}

void MapSky::Draw()
{
	VECTOR3 save = GameDevice()->m_vLightDir;
	GameDevice()->m_vLightDir = VECTOR3(0, 0, 0);  // 陰影のない描画にする
	GameDevice()->m_pD3D->SetZBuffer(false); // Zバッファを無効化

	Object3D::Draw();

	GameDevice()->m_pD3D->SetZBuffer(true); // Zバッファを有効化
	GameDevice()->m_vLightDir = save;		// 陰影のある描画に戻す
}