#pragma once
#include "SceneBase.h"

class OverScene : public SceneBase
{
public:
	OverScene();
	~OverScene();
	void Update() override;
	void Draw() override;
};
