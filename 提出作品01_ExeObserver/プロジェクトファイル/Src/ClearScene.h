#pragma once
#include "SceneBase.h"

class ClearScene : public SceneBase
{
public:
	ClearScene();
	~ClearScene();
	void Update() override;
	void Draw() override;

private:
	CSpriteImage* clearimage;
	CSprite* sprite;
};
