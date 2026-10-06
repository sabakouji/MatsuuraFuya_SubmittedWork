#include "ClearDraw.h"
#include "GameMain.h"

ClearDraw::ClearDraw()
{
	timer = 0;

	image = new CSpriteImage("Data/Image/CLEAR.png");

}

ClearDraw::~ClearDraw()
{
	SAFE_DELETE(image);
	SAFE_DELETE(sprite);
}

void ClearDraw::Update()
{
}

void ClearDraw::Draw()
{
	sprite->Draw(image, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}