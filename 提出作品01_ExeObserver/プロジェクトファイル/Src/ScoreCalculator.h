#pragma once
#include <string>

class Executor;
class Camera;

class ScoreCalculator {
public:
  static void CalculateScore(Executor *executor, Camera *camera, int &outScore,
                             char &outRank);
};
