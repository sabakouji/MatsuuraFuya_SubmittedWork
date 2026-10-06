// 6 [EnemyManager] Constructor
// 79 [MeshList] Get mesh by name
// 89 [Spawn] Spawn enemy from script (supports patrol command)

#include "EnemyManager.h"
#include "EnemyGolem.h"
#include "EnemyMecha.h"
#include <cmath>

EnemyManager::EnemyManager() {
  // EnemyBase基底でSetTag("Enemy")されるが、Manager自身は敵ではないためタグを除去
  SetTag("");

  ObjectManager::DontDestroy(this);
  ObjectManager::SetVisible(this, false);

  mesh = nullptr;
  meshCol = nullptr;

  meshstruct ms = {};

  // Golem
  meshList.push_back(ms);
  meshList.back().name = "Golem";
  meshList.back().mesh = new CFbxMesh();
  meshList.back().mesh->Load("Data/Char/Golem/golem.mesh");
  meshList.back().mesh->LoadAnimation(aIdle, "Data/Char/Golem/golem_stand.anmx",
                                      true);
  meshList.back().mesh->LoadAnimation(
      aRun, "Data/Char/Golem/golem_walk_RAXZ.anmx", true, eRootAnimXZ);
  meshList.back().mesh->LoadAnimation(
      aAttack1, "Data/Char/Golem/golem_attack.anmx", false);
  meshList.back().mesh->LoadAnimation(aDead, "Data/Char/Golem/golem_die.anmx",
                                      false);

  // EnemyMecha
  meshList.push_back(ms);
  meshList.back().name = "EnemyMecha";
  meshList.back().mesh = new CFbxMesh();
  meshList.back().mesh->Load("Data/Char/EnemyMecha/EnemyMecha.mesh");
  meshList.back().mesh->LoadAnimation(aIdle, "Data/Char/EnemyMecha/EnemyMecha_Idle.anmx", true);
  meshList.back().mesh->LoadAnimation(aRun, "Data/Char/EnemyMecha/EnemyMacha_Walk.anmx", true);
  meshList.back().mesh->LoadAnimation(aAttack1, "Data/Char/EnemyMecha/EnemyMecha_Kick.anmx", false); 
  meshList.back().mesh->LoadAnimation(aDead, "Data/Char/EnemyMecha/EnemyMecha_Dying.anmx", false);
}

EnemyManager::~EnemyManager() {
  for (meshstruct &ms : meshList) {
    SAFE_DELETE(ms.mesh);
  }
}

CFbxMesh *EnemyManager::MeshList(std::string str) {
  for (meshstruct &ms : meshList) {
    if (str == ms.name)
      return ms.mesh;
  }
  MessageBox(nullptr, "EnemyManager::MeshList()",
             _T("指定されたメッシュはリストにありません"), MB_OK);
  return nullptr;
}

void EnemyManager::Spawn(TextReader *txt, int n) {
  const int headColumn = 1;

  navigationMap.clear();
  navigationMap.shrink_to_fit();

  std::string param1 = txt->GetString(n, headColumn);
  if (param1.find("patrol(") == 0) {
    // patrol(r) mode
    float radius = 10.0f;
    sscanf_s(param1.c_str(), "patrol(%f)", &radius);

    // Center pos is at col 2,3,4 (index 2,3,4 relative to 0? No, headColumn+1
    // is index 2)
    VECTOR3 center;
    center.x = txt->GetFloat(n, headColumn + 1);
    center.y = txt->GetFloat(n, headColumn + 2);
    center.z = txt->GetFloat(n, headColumn + 3);

    // Generate random points
    for (int i = 0; i < 5; i++) {
      float angle = (float)rand() / RAND_MAX * 3.14159265f * 2.0f;
      float r = (float)rand() / RAND_MAX * radius;
      VECTOR3 p = center + VECTOR3(cos(angle) * r, 0, sin(angle) * r);
      navigationMap.emplace_back(p);
    }
  } else {
    // Normal mode
    int num = (txt->GetColumns(n) - headColumn) / 3;
    for (int i = 0; i < num; i++) {
      VECTOR3 pos;
      pos.x = txt->GetFloat(n, headColumn + i * 3);
      pos.y = txt->GetFloat(n, headColumn + i * 3 + 1);
      pos.z = txt->GetFloat(n, headColumn + i * 3 + 2);
      navigationMap.emplace_back(pos);
    }
  }

  std::string str = txt->GetString(n, 0);

  // ---------------------------------------

  if (str == "EnemyGolem") {
    EnemyGolem *enmObj = Instantiate<EnemyGolem>();
    enmObj->MakeNavigationMap(navigationMap);
  }

  if (str == "EnemyMecha") {
    EnemyMecha *enmObj = Instantiate<EnemyMecha>();
    enmObj->MakeNavigationMap(navigationMap);
  }
}
