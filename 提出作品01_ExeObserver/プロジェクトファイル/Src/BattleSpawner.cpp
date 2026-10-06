// 10行目 [BattleSpawner] コンストラクタ
// 22行目 [MakeSpawner] パラメータ読み込みと初期化
// 35行目 [Update] 更新処理（エネミースポーンとクリア判定）
// 66行目 [Draw] 描画（今回は何も描画しない）

#include "BattleSpawner.h"
#include "EnemyMecha.h"
#include "SceneManager.h"
#include <cmath>
#include <cstdlib>

BattleSpawner::BattleSpawner()
    : maxEnemies(5), spawnIntervalSeconds(5.0f), killTarget(10),
      baseSpawnPos(0.0f, 0.0f, 0.0f), spawnRadius(0.0f), patrolRadius(0.0f),
      spawnTimer(0.0f), totalSpawned(0), currentKilled(0) {}

BattleSpawner::~BattleSpawner() {}

void BattleSpawner::MakeSpawner(TextReader *txt, int line) {
  maxEnemies = txt->GetInt(line, 1);
  spawnIntervalSeconds = txt->GetFloat(line, 2);
  killTarget = txt->GetInt(line, 3);
  baseSpawnPos.x = txt->GetFloat(line, 4);
  baseSpawnPos.y = txt->GetFloat(line, 5);
  baseSpawnPos.z = txt->GetFloat(line, 6);
  if (txt->GetColumns(line) > 7) {
    spawnRadius = txt->GetFloat(line, 7);
  } else {
    spawnRadius = 0.0f;
  }

  patrolRadius = 0.0f;
  for (unsigned int c = 7; c < txt->GetColumns(line); c++) {
    std::string param = txt->GetString(line, c);
    if (param.find("patrol(") == 0) {
      sscanf_s(param.c_str(), "patrol(%f)", &patrolRadius);
    }
  }
  // 最初はすぐに湧くようにする
  spawnTimer = spawnIntervalSeconds;
}

void BattleSpawner::Update() {
  std::list<EnemyMecha *> enemies =
      ObjectManager::FindGameObjects<EnemyMecha>();
  int currentAlive = (int)enemies.size();

  currentKilled = totalSpawned - currentAlive;

  if (currentKilled >= killTarget) {
    SceneManager::ChangeScene("ClearScene");
    return;
  }

  if (currentAlive < maxEnemies) {
    spawnTimer += SceneManager::DeltaTime();
    if (spawnTimer >= spawnIntervalSeconds) {
      spawnTimer = 0.0f;

      EnemyMecha *enmObj = Instantiate<EnemyMecha>();

      // ランダム座標計算
      float angle = (float)rand() / RAND_MAX * 3.14159265f * 2.0f;
      float r = (float)rand() / RAND_MAX * spawnRadius;
      VECTOR3 spawnPosOffset(cos(angle) * r, 0.0f, sin(angle) * r);
      VECTOR3 spawnPos = baseSpawnPos + spawnPosOffset;

      std::vector<VECTOR3> map;
      if (patrolRadius > 0.0f) {
        for (int i = 0; i < 5; i++) {
          float pa = (float)rand() / RAND_MAX * 3.14159265f * 2.0f;
          float pr = (float)rand() / RAND_MAX * patrolRadius;
          VECTOR3 p(spawnPos.x + cos(pa) * pr, spawnPos.y,
                    spawnPos.z + sin(pa) * pr);
          map.push_back(p);
        }
      } else {
        map.push_back(spawnPos);
      }
      enmObj->MakeNavigationMap(map);

      totalSpawned++;
    }
  }
}

void BattleSpawner::Draw() {}
