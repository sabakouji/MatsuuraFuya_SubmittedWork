#include "Object2D.h"
#include "ResourceManager.h"
#include "GameMain.h"


void TransformData::CalcDrawMatrix()
{
	VECTOR2 scr = GameDevice()->Scroll;    // スクロール位置

	if (rotation == 0)
	{
		drawMatrix = XMMatrixScaling(scale.x, scale.y, 1) * XMMatrixTranslation(position.x-center.x*scale.x - scr.x, position.y-center.y*scale.y - scr.y, 0);
	}
	else {
		drawMatrix = XMMatrixScaling(scale.x, scale.y, 1) * XMMatrixRotationZ(rotation * Deg2Rad) * XMMatrixTranslation(position.x - scr.x, position.y - scr.y, 0);
		drawMatrix = XMMatrixTranslation(-center.x, -center.y, 0) * drawMatrix;
	}
}

bool Object2D::CreateSprite(CSpriteImage* image, const VECTOR4 pattern)
{
	return CreateSprite(image, pattern.x, pattern.y, pattern.z, pattern.w);
}
bool Object2D::CreateSprite(CSpriteImage* image, const DWORD& srcX, const DWORD& srcY, const DWORD& srcwidth, const DWORD& srcheight)
{
	sprite = new CSprite(image, srcX, srcY, srcwidth, srcheight);
	if (sprite == nullptr) return false;

	// 移動や回転・拡縮の中心点をここで決める。後から変更も可
	// (0,0)で左上
	//transform.center.x = 0;
	//transform.center.y = 0;

	// (srcwidth/2,srcheight/2)でスプライト中心。
	//transform.center.x = srcwidth / 2;
	//transform.center.y = srcheight / 2;

	// (srcwidth/2,srcheight)で下部中心位置
	transform.center.x = srcwidth / 2;
	transform.center.y = srcheight;

	return true;
}

void Object2D::Draw()
{
	VECTOR2 scr = GameDevice()->Scroll;    // スクロール位置

	// 画面外を表示しない
	if (transform.position.x + (sprite->GetSrcWidth() - transform.center.x) * transform.scale.x - scr.x < 0)  return;
	if (transform.position.x - transform.center.x * transform.scale.x - scr.x > WINDOW_WIDTH)  return;
	if (transform.position.y + (sprite->GetSrcHeight() - transform.center.y) * transform.scale.y - scr.y < 0)  return;
	if (transform.position.y - transform.center.y * transform.scale.y - scr.y > WINDOW_HEIGHT)  return;

	transform.CalcDrawMatrix();
	sprite->Draw(transform.drawMatrix);
}
