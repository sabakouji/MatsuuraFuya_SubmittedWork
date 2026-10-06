#include "MapManager.h"

namespace {
	const int   MaxDivide = 4096;
	const float MaxHeight = 0.08f;
};

MapStage::MapStage()
{
	ObjectManager::SetDrawOrder(this, 10);   // ステージは少し早めに描画する

	mesh = new CFbxMesh();

}

MapStage::~MapStage()
{
	SAFE_DELETE(mesh);
	SAFE_DELETE(meshCol);	   // Eventクラスで生成されることがあるので削除は必須
}

void MapStage::MakeStageMap(TextReader* txt, int n)
{
	VECTOR3 pos, rot;
	int colmn = txt->GetColumns(n);
	pos.x = txt->GetFloat(n, 1);
	pos.y = txt->GetFloat(n, 2);
	pos.z = txt->GetFloat(n, 3);
	rot.x = 0;
	rot.y = txt->GetFloat(n, 4);
	rot.z = 0;
	transform.position = pos;
	transform.rotation = rot * DegToRad;	// ラジアン角にする

	std::string tag = txt->GetString(n, 5);	   // タグ名
	SetTag(tag);

	std::string meshName = txt->GetString(n, 6);
	mesh->Load(meshName.c_str());

	if (colmn > 7)
	{
		std::string colName = txt->GetString(n, 7);
		if (colName != "") ObjectManager::FindGameObject<MapManager>()->ColMesh()->AddFbxLoad(colName.c_str(), transform.matrix());
	}
	if (colmn > 8)	 // ディスプレイスメントマッピング
	{
		std::string disName = txt->GetString(n, 8);
		if (disName == "Displace")
		{
			mesh->m_iMaxDevide = MaxDivide;
			mesh->m_fHeightMax = MaxHeight;
		}
	}
}

