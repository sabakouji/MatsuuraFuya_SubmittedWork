#pragma once
#include "SceneBase.h"

/// <summary>
/// ゲームオーバーシーンのクラス
/// </summary>
class OverScene : public SceneBase
{
public:
	OverScene();
	~OverScene();
	void Update() override;
	void Draw() override;
private:
	CSpriteImage* image;
	CSprite* sprite;
};
