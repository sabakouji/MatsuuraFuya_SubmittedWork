#include "MapManager.h"


MapManager::MapManager()
{
	ObjectManager::SetVisible(this, false);		// 自体は表示しない

	colMesh = new CCollision();     // コリジョンマップの生成
}

MapManager::~MapManager()
{
	SAFE_DELETE(colMesh);

}
void MapManager::MakeMap(TextReader* txt, int n)
{
	std::string str = txt->GetString(n, 0);

	if (str == "MapStage")
	{
		MapStage* msObj = Instantiate<MapStage>();
		msObj->MakeStageMap(txt, n);
	}
	else if (str == "MapMove")
	{
		MapMove* msObj = Instantiate<MapMove>();
		msObj->MakeMoveMap(txt, n);
	}
	else if (str == "MapSky")
	{
		MapSky* msObj = Instantiate<MapSky>();
		msObj->MakeSkyMap(txt, n);
	}
	else if (str == "MapWave")
	{
		MapWave* msObj = Instantiate<MapWave>();
		msObj->MakeWaveMap(txt, n);
	}
	else if (str == "MapTree")
	{
		MapTree* msObj = Instantiate<MapTree>();
		msObj->MakeTreeMap(txt, n);
	}
}

bool MapManager::IsCollisionLay(const VECTOR3& startIn, const VECTOR3& endIn, VECTOR3& hit, VECTOR3& normal)
{
	bool ret = false, retMove = false;

	// 移動マップとの接触判定
	std::list<MapMove*> mml = ObjectManager::FindGameObjects<MapMove>();
	for (MapMove*& mm : mml)
	{
		if (mm->ColMoveMesh() && mm->ActiveOn())
		{
			retMove = mm->ColMoveMesh()->IsCollisionLay(startIn, endIn, hit, normal);
			if (retMove == true) break;   // 移動マップと接触したとき
		}
	}

	// 通常のマップとの接触判定
	if (colMesh)
	{
		ret = colMesh->IsCollisionLay(startIn, endIn, hit, normal);
	}

	if (retMove == true)  // 移動マップと接触していたとき
	{
		return retMove;
	}
	else {
		return ret;
	}
}
bool MapManager::IsCollisionSphere(const VECTOR3& startIn, const VECTOR3& endIn, const float& radius, VECTOR3& hit, VECTOR3& normal)
{
	bool ret = false, retMove = false;

	// 移動マップとの接触判定
	std::list<MapMove*> mml = ObjectManager::FindGameObjects<MapMove>();
	for (MapMove*& mm : mml)
	{
		if (mm->ColMoveMesh() && mm->ActiveOn())
		{
			retMove = mm->ColMoveMesh()->IsCollisionSphere(startIn, endIn, radius, hit, normal);
			if (retMove == true) break;   // 移動マップと接触したとき
		}
	}

	// 通常のマップとの接触判定
	if (colMesh)
	{
		ret = colMesh->IsCollisionSphere(startIn, endIn, radius, hit, normal);
	}

	if (retMove == true)  // 移動マップと接触していたとき
	{
		return retMove;
	}
	else {
		return ret;
	}
}
bool MapManager::IsCollisionMove(const VECTOR3& positionOld, VECTOR3& position, float radius)
{
	VECTOR3 hit, normal;
	return IsCollisionMove(positionOld, position, hit, normal, radius);
}
bool MapManager::IsCollisionMove(const VECTOR3& positionOld, VECTOR3& position, VECTOR3& hit, VECTOR3& normal, float radius)
{
	bool ret = false, retMove =false;

	// 移動マップとの接触判定と移動
	std::list<MapMove*> mml = ObjectManager::FindGameObjects<MapMove>();
	for (MapMove*& mm : mml)
	{
		if (mm->ColMoveMesh() && mm->ActiveOn())
		{
			retMove = mm->ColMoveMesh()->IsCollisionMove(positionOld, position, hit, normal, radius);
			if (retMove == true) break;   // 移動マップと接触したとき
		}
	}

	// 通常のマップとの接触判定と移動
	if (colMesh)
	{
		ret = colMesh->IsCollisionMove(positionOld, position, hit, normal, radius);
	}

	if (retMove == true)  // 移動マップと接触していたとき
	{
		return retMove;
	}
	else {
		return ret;
	}
}

CollRet  MapManager::IsCollisionMoveGravity(const VECTOR3& positionOld, VECTOR3& position, float radius)
{
	VECTOR3 hit, normal;
	return IsCollisionMoveGravity(positionOld, position, hit, normal, radius);
}
CollRet  MapManager::IsCollisionMoveGravity(const VECTOR3& positionOld, VECTOR3& position, VECTOR3& hit, VECTOR3& normal, float radius)
{
	CollRet ret = clError, retMove = clError;

	// 移動マップとの接触判定と移動
	std::list<MapMove*> mml = ObjectManager::FindGameObjects<MapMove>();
	for (MapMove* &mm : mml)
	{
		if (mm->ColMoveMesh() && mm->ActiveOn())
		{
			retMove = mm->ColMoveMesh()->IsCollisionMoveGravity(positionOld, position, hit, normal, radius);
			if (retMove == clMove || retMove == clLand)
				break;   // 移動マップと接触したとき
		}
	}

	// 通常のマップとの接触判定と移動
	if (colMesh)
	{
		ret = colMesh->IsCollisionMoveGravity(positionOld, position, hit, normal, radius);
	}

	if (retMove == clMove || retMove == clLand)  // 移動マップと接触していたとき
	{
		return retMove;
	}
	else {
		return ret;
	}
}
