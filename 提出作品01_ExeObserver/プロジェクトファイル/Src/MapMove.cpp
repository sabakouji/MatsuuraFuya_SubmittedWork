#include "MapManager.h"

namespace {
	const int   MaxDivide = 4096;
	const float MaxHeight = 0.08f;
};

MapMove::MapMove()
{
	ObjectManager::SetDrawOrder(this, 10);   // 少し早めに描画する

	moveFlag = 0;				// 移動区分    1:平行移動　2:回転　3:拡大縮小
	activeOn = true;			// 処理・表示ON   false:処理・表示なし　true:処理・表示あり
	moveOn = false;				// 移動ON      false:移動停止　true:移動実行
	up = VECTOR3(0,0,0);		// 移動増分
	minOffset = VECTOR3(-9999, -9999, -9999);	// 移動増分最小値
	maxOffset = VECTOR3(9999, 9999, 9999);	// 移動増分最大値
	changeFlag = 0;		// 移動が限界値に到達して方向が入れ替わるとき(出力)　0:移動中　1:方向チェンジ
	loop = 1;			// 移動を繰り返すか  0:チェンジの箇所で停止し移動ONを0:移動停止に戻す。 1:移動を繰り返す

	mesh = new CFbxMesh();
	colMoveMesh = new CCollision;
}

MapMove::~MapMove()
{
	SAFE_DELETE(mesh);
	SAFE_DELETE(colMoveMesh);
}

void MapMove::MakeMoveMap(TextReader* txt, int n)
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
	transform.rotation = rot * DegToRad;   // ラジアン角にする

	std::string meshName = txt->GetString(n, 18);
	mesh->Load(meshName.c_str());

	if (colmn > 19)
	{
		std::string colName = txt->GetString(n, 19);
		if (colName != "")
		{
			colMoveMesh->AddFbxLoad(colName.c_str());
			colMoveMesh->InitWorldMatrix(transform.matrix());
		}
	}
	if (colmn > 20)	 // ディスプレイスメントマッピング
	{
		std::string disName = txt->GetString(n, 20);
		if (disName == "Displace")
		{
			mesh->m_iMaxDevide = MaxDivide;
			mesh->m_fHeightMax = MaxHeight;
		}
	}

	transformBase = transform;	// 基準位置
	velocity = VECTOR3(0, 0, 0);
	tag = txt->GetString(n, 5);	  // タグ名をつける
	SetTag(tag);
	if (txt->GetString(n, 7) == "Rot")
	{
		moveFlag = 2;				// 移動区分    1:平行移動　2:回転　3:拡大縮小
	}
	else if(txt->GetString(n, 7) == "Scale")
	{
		moveFlag = 3;				// 移動区分    1:平行移動　2:回転　3:拡大縮小
	}
	else {
		moveFlag = 1;				// 移動区分    1:平行移動　2:回転　3:拡大縮小
	}
	if( txt->GetString(n, 6) == "ActiveOff"){
		activeOn = false;			// 処理・表示ON   false:処理・表示なし　true:処理・表示あり
	}
	else  if( txt->GetString(n, 6) == "MoveOn"){
		moveOn = true;				// 移動ON      false:移動停止　true:移動実行
	}

	VECTOR3 work;
	work.x = txt->GetFloat(n, 8);
	work.y = txt->GetFloat(n, 9);
	work.z = txt->GetFloat(n, 10);
	up = work;		// 増分

	work.x = txt->GetFloat(n, 11);
	work.y = txt->GetFloat(n, 12);
	work.z = txt->GetFloat(n, 13);
	minOffset = work;	// 増分最小値

	work.x = txt->GetFloat(n, 14);
	work.y = txt->GetFloat(n, 15);
	work.z = txt->GetFloat(n, 16);
	maxOffset = work;	// 増分最大値

	if (txt->GetString(n, 17) != "Loop")
	{
		loop = 0;
	}
	else {
		loop = 1;
	}
}

void MapMove::Update()
{
	colMoveMesh->SaveWorldMatrix();   // ワールドマトリックス一つ前の待避

	if (colMoveMesh != nullptr && moveOn)   // 移動マップの更新処理
	{
		changeFlag = 0;      // チェンジフラグのリセット
		if (moveFlag == 1)   // 平行移動の処理
		{
			velocity += up * 60 * SceneManager::DeltaTime();    // 移動増分の処理
			if (up.x != 0)  // 左右に動かす
			{
				if (checkLimitAndSet(minOffset.x, maxOffset.x, velocity.x, up.x) == 1) changeFlag = 1;
			}
			if (up.y != 0)  // 上下に動かす
			{
				if (checkLimitAndSet(minOffset.y, maxOffset.y, velocity.y, up.y) == 1) changeFlag = 1;
			}
			if (up.z != 0)  // 前後に動かす
			{
				if (checkLimitAndSet(minOffset.z, maxOffset.z, velocity.z, up.z) == 1) changeFlag = 1;
			}
			transform.position = transformBase.position+ velocity;
		}
		else if (moveFlag == 2) {   // 回転の処理
			velocity += up * 60 * SceneManager::DeltaTime();    // 回転増分の処理
			if (up.x != 0)  // Ｘ回転
			{
				if (checkLimitAndSet(minOffset.x, maxOffset.x, velocity.x, up.x) == 1) changeFlag = 1;
			}
			else if (up.y != 0)  // Ｙ回転
			{
				if (checkLimitAndSet(minOffset.y, maxOffset.y, velocity.y, up.y) == 1) changeFlag = 1;
			}
			else if (up.z != 0)  // Ｚ回転
			{
				if (checkLimitAndSet(minOffset.z, maxOffset.z, velocity.z, up.z) == 1) changeFlag = 1;
			}
			transform.rotation = transformBase.rotation + velocity * DegToRad;   // ラジアン角にする
		}
		else if (moveFlag == 3) {   // 拡大縮小の処理
			velocity += up * 60 * SceneManager::DeltaTime();    // 拡縮増分の処理
			VECTOR3 vScaleNow = transformBase.scale + velocity;     // 現在の拡大率を得る
			// 規定の拡大縮小に達したか
			if (up.x != 0 )
			{
				if (checkLimitAndSet(minOffset.x, maxOffset.x, vScaleNow.x, up.x) == 1) changeFlag = 1;
			}
			if (up.y != 0 )
			{
				if (checkLimitAndSet(minOffset.y, maxOffset.y, vScaleNow.y, up.y) == 1) changeFlag = 1;
			}
			if (up.z != 0 )
			{
				if (checkLimitAndSet(minOffset.z, maxOffset.z, vScaleNow.z, up.z) == 1) changeFlag = 1;
			}
			velocity = vScaleNow - transformBase.scale;
			transform.scale = transformBase.scale + velocity;
		}
		// ループをしないときの停止判断。反転のタイミングで停止する
		if (loop == 0 && changeFlag == 1)
		{
			moveOn = false;    // 移動しない
		}
		colMoveMesh->SetWorldMatrix(transform.matrix());
	}
}

int  MapMove::checkLimitAndSet(const float minOff, const float maxOff, float& velocityIO, float& upIO)
{
	int change = 0;

	if (velocityIO < minOff )
	{
		velocityIO = minOff;
		if (upIO < 0) upIO *= -1;
		change = 1;
	}else if( velocityIO > maxOff)
	{
		velocityIO = maxOff;
		if (upIO > 0) upIO *= -1;
		change = 1;
	}
	return change;
}

void MapMove::Draw()
{
	if (!activeOn) return;
	Object3D::Draw();
}
