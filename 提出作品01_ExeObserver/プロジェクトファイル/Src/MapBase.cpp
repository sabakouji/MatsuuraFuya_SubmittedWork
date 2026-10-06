#include "MapBase.h"

MapBase::MapBase()
{
	colMoveMesh = nullptr;
}

MapBase::~MapBase()
{
}
void MapBase::CreateMeshcolliderFromMesh(CFbxMesh* mesh)
{
	meshCol = new MeshCollider();
	meshCol->MakeFromMesh(mesh);
}
