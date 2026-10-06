#pragma once
#include "Object3D.h"

class TitleDraw : public Object3D
{
public:
	TitleDraw();
	~TitleDraw();
	void Update() override;
	void Draw() override;
private:
	float timer;
	CSpriteImage* image;
	CSprite* sprite;
};
