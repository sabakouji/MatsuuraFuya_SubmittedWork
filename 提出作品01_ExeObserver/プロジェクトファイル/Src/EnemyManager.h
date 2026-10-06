#pragma once
#include "ECS/World.h"
#include "Animator.h"
#include "TextReader.h"

#include "EnemyBase.h"
#include <string>
#include <list>

#include "EnemyGolem.h"
#include "EnemyMecha.h"

class EnemyManager : public EnemyBase
{
private:
	struct meshstruct{
		std::string name;
		CFbxMesh* mesh;
	};

public:
	EnemyManager();
	~EnemyManager();

	CFbxMesh* MeshList(std::string str);

	void Spawn(TextReader* txt, int n);

private:
	std::list<meshstruct> meshList;
};
