#pragma once
#include "GameObject.h"

class PauseObject : public GameObject {
public:
  PauseObject();
  ~PauseObject();

  void Start() override;
  void Update() override;
  void Draw() override;
};
