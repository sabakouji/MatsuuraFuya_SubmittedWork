#include "TitleDraw.h"
#include "GameMain.h"

TitleDraw::TitleDraw()
{
	timer = 0;

	image = new CSpriteImage("Data/Image/TitleBG.png");
	sprite = new CSprite;
}

TitleDraw::~TitleDraw()
{
	SAFE_DELETE(image);
	SAFE_DELETE(sprite);
}

void TitleDraw::Update()
{
}

void TitleDraw::Draw()
{
}