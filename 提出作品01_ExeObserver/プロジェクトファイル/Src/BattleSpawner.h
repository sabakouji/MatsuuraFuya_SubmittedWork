// [BattleSpawner] バトルステージ用エネミースポナー
#pragma once
#include "GameObject.h"
#include "TextReader.h"
#include <vector>

class BattleSpawner : public GameObject {
public:
  BattleSpawner();
  virtual ~BattleSpawner();

  void MakeSpawner(TextReader *txt, int line);

  virtual void Update() override;
  virtual void Draw() override;

private:
  int maxEnemies;
  float spawnIntervalSeconds;
  int killTarget;
  VECTOR3 baseSpawnPos;
  float spawnRadius;
  float patrolRadius;

  float spawnTimer;
  int totalSpawned;
  int currentKilled;
};
