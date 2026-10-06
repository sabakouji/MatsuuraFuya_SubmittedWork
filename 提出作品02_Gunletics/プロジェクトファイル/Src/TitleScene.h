#pragma once
#include "SceneBase.h"

/// <summary>
/// タイトルシーンのクラス
/// </summary>
class TitleScene : public SceneBase
{
public:
	TitleScene();
	~TitleScene();
	void Update() override;
	void Draw() override;
private:
	CSpriteImage* image;
	CSprite* sprite;
};
