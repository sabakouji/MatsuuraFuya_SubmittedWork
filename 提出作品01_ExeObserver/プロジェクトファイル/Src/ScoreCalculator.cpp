
#include "ScoreCalculator.h"
#include "Camera.h"
#include "Executor.h"
#include <DirectXMath.h>
#include <cmath>

using namespace DirectX;

void ScoreCalculator::CalculateScore(Executor *executor, Camera *camera,
                                     int &outScore, char &outRank) {
  outScore = 0;
  outRank = 'E';

  if (!executor || !camera) {
    return;
  }

  VECTOR3 camPos = camera->Position();

  VECTOR3 executorPos = executor->Position();
  float dist = (executorPos - camPos).Length();

  // 距離スコア: 近すぎず遠すぎず (3.0f ~ 8.0f が適正とする)
  if (dist >= 2.0f && dist <= 10.0f) {
    outScore += 1000;
  } else if (dist < 2.0f) // 近すぎ
  {
    outScore += 500;
  } else // 遠すぎ
  {
    outScore += 200;
  }

  // Executorが攻撃中なら高得点
  if (executor->IsAttacking()) {
    outScore += 3000;

    // スキルごとのボーナス
    // (本来はExecutorから現在実行中のスキルIDなどを取得して分岐)
    int skillScore = executor->GetCurrentUseSkillScore();
    outScore += skillScore;
  }

  // Executorの体力が少ないほど高得点
  std::string runningNode = executor->GetCurrentRunningNode();
  if (runningNode.find("Skill") != std::string::npos) {
    outScore += 500;
  }

  // ランク判定
  if (outScore >= 8000)
    outRank = 'S';
  else if (outScore >= 5000)
    outRank = 'A';
  else if (outScore >= 3000)
    outRank = 'B';
  else if (outScore >= 1000)
    outRank = 'C';
  else if (outScore >= 500)
    outRank = 'D';
  else
    outRank = 'E';
}
