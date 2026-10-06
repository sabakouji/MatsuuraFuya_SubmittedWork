#include "DisplayInfo.h"
#include "DataCarrier.h"
#include "PlayScene.h"
#include "Player.h"

namespace {
	const int ScoreMax = 5000;   // スコアバーの表示上のスコアの最大値
	const VECTOR4 LifeUI = VECTOR4{ 0,0,222,434 };
	const int fullsegment = 86;
	const int sourceY = 139;
	const int baseDestY = WINDOW_HEIGHT - 150 + 27;
	const int segmentwidth = 32;
}

DisplayInfo::DisplayInfo(CSpriteImage* inimage)
{
	image = inimage;
	sprite = new CSprite();

	SetDrawOrder(-100);	   // 一番最後に描画する
}

DisplayInfo::~DisplayInfo()
{
	SAFE_DELETE(sprite);
}

void DisplayInfo::Update()
{
}

void DisplayInfo::Draw()
{
	float h = 0, m = 0;
	TCHAR str[256];
	int   DestX, DestY;

	// ステータスバーの表示
	Player* obj = ObjectManager::FindGameObject<Player>();
	DataCarrier* data = ObjectManager::FindGameObject<DataCarrier>();

	h = (float)obj->HpdivMax();
	if (h < 0) h = 0;
	m = (float)obj->MpdivMax();
	if (m < 0) m = 0;

	PLHP = obj->Hp();
	hpsegment = obj->MaxHP() / 3;

	DestX = 10;
	DestY = WINDOW_HEIGHT - 150;
	if (obj->isDead == true)
	{
		HPwi = 271;
	}
	else
	{
		HPwi = 0;
	}
	HPX = 270;
	HPY = 138;
	sprite->Draw(image, DestX, DestY, HPwi, 0, HPX, HPY);

	int segment3X = DestX + 39 + 71 + 70;
	currentheigth3 = 0;
	currentSouce3 = sourceY;
	currentDest3 = baseDestY;
	if (PLHP > hpsegment * 2)
	{
		double remainingHPsegment = static_cast<double>(PLHP - (hpsegment * 2));
		if (remainingHPsegment < 0)remainingHPsegment = 0;

		currentheigth3 = static_cast<int>((remainingHPsegment / hpsegment) * fullsegment);
		if (currentheigth3 < 0)currentheigth3 = 0;

		int yoffset = fullsegment - currentheigth3;
		currentSouce3 = sourceY + yoffset;
		currentDest3 = baseDestY + yoffset;
	}
	else
	{
		currentheigth3 = 0;
	}
	if (currentheigth3 > 0)
	{
		sprite->Draw(image, segment3X, currentDest3, 0, currentSouce3, segmentwidth, currentheigth3);
	}

	int segment2X = DestX + 39 + 71;
	currentheigth2 = 0;
	currentSouce2 = sourceY;
	currentDest2 = baseDestY;
	if (PLHP > hpsegment * 1)
	{
		if (PLHP > hpsegment * 2)
		{
			currentheigth2 = fullsegment;
		}
		else
		{
			double remainingHPforsegment = static_cast<double>(PLHP - (hpsegment * 1));
			if (remainingHPforsegment < 0) remainingHPforsegment = 0;

			currentheigth2 = static_cast<int>((remainingHPforsegment / hpsegment) * fullsegment);
			if (currentheigth2 < 0) currentheigth2 = 0;

			int yoffset = fullsegment - currentheigth2;
			currentSouce2 = sourceY + yoffset;
			currentDest2 = baseDestY + yoffset;
		}
	}
	if (currentheigth2 > 0)
	{
		sprite->Draw(image, segment2X, currentDest2, 0, currentSouce2, segmentwidth, currentheigth2);
	}

	int segment1X = DestX + 39;
	currentheigth1 = 0;
	currentSouce1 = sourceY;
	currentDest1 = baseDestY;
	if (PLHP > 0)
	{
		if (PLHP > hpsegment * 1)
		{
			currentheigth1 = fullsegment;
		}
		else
		{
			double remainingHPforsegment = static_cast<double>(PLHP);
			if (remainingHPforsegment > hpsegment)remainingHPforsegment = hpsegment;

			currentheigth1 = static_cast<int>((remainingHPforsegment / hpsegment) * fullsegment);
			if (currentheigth1 < 0)currentheigth1 = 0;

			int yoffset = fullsegment - currentheigth1;
			currentSouce1 = sourceY + yoffset;
			currentDest1 = baseDestY + yoffset;
		}
	}
	if (currentheigth1 > 0)
	{
		sprite->Draw(image, segment1X, currentDest1, 0, currentSouce1, segmentwidth, currentheigth1);
	}

	//sprite->Draw(image, DestX, DestY, 0, 139, 32, 86);

	/*
	_stprintf_s(str, _T("%d"), obj->Num());
	GameDevice()->m_pFont->Draw(DestX + 6, DestY + 15, str, 16, RGB(255, 0, 0));
	_stprintf_s(str, _T("%06d"), obj->Hp());
	GameDevice()->m_pFont->Draw(DestX + 26, DestY + 16, str, 12, RGB(0, 0, 0));
	*/

	DestX = WINDOW_WIDTH - 220;
	DestY = WINDOW_HEIGHT - 90;
	sprite->Draw(image, DestX, DestY, 33, 139, 198, 85);
	int mg = obj->Getmg();
	int weapon = obj->GetCurrentWeapon();

	if (mg < 0)mg = 0;
	switch (weapon)
	{
	case Weapon_Shot:
		GameDevice()->m_pFont->Draw(WINDOW_WIDTH - 135, WINDOW_HEIGHT - 70, _T("通常"), 47, RGB(0, 255, 0), 1.0f, _T("HGP創英ﾌﾟﾚｾﾞﾝｽEB"));
		_stprintf_s(str, _T("%d"), mg);
		GameDevice()->m_pFont->Draw(WINDOW_WIDTH - 210, WINDOW_HEIGHT - 65, str, 48, RGB(0, 255, 0));
		break;
	case Weapon_Penetrate:
		GameDevice()->m_pFont->Draw(WINDOW_WIDTH - 135, WINDOW_HEIGHT - 70, _T("貫通"), 47, RGB(0, 255, 0), 1.0f, _T("HGP創英ﾌﾟﾚｾﾞﾝｽEB"));
		_stprintf_s(str, _T("%d"), mg);
		GameDevice()->m_pFont->Draw(WINDOW_WIDTH - 198, WINDOW_HEIGHT - 65, str, 48, RGB(0, 255, 0));
		break;
	case Weapon_Grenade:
		GameDevice()->m_pFont->Draw(WINDOW_WIDTH - 135, WINDOW_HEIGHT - 70, _T("爆弾"), 47, RGB(0, 255, 0), 1.0f, _T("HGP創英ﾌﾟﾚｾﾞﾝｽEB"));
		_stprintf_s(str, _T("%d"), mg);
		GameDevice()->m_pFont->Draw(WINDOW_WIDTH - 198, WINDOW_HEIGHT - 65, str, 48, RGB(0, 255, 0));
		break;
	}

	int second = obj->Getsecond();
	data->Setsecond(second);
	_stprintf_s(str, _T("%d"), second);
	GameDevice()->m_pFont->Draw(WINDOW_WIDTH - 60, 20, str, 48, RGB(0, 255, 0));
	int tensecond = obj->GetTenSecond();
	data->Settensecond(tensecond);
	_stprintf_s(str, _T("%d"), tensecond);
	GameDevice()->m_pFont->Draw(WINDOW_WIDTH - 85, 20, str, 48, RGB(0, 255, 0));
	int minute = obj->Getminute();
	data->Setminute(minute);
	_stprintf_s(str, _T("%d"), minute);
	GameDevice()->m_pFont->Draw(WINDOW_WIDTH - 130, 20, str, 48, RGB(0, 255, 0));
	GameDevice()->m_pFont->Draw(WINDOW_WIDTH - 108, 15, _T("："), 48, RGB(0, 255, 0), 1.0f, _T("HGP創英ﾌﾟﾚｾﾞﾝｽEB"));

	//sprite->Draw(image, DestX + 59, DestY + 6, 59, 186, (int)(144 * m), 6);
	//_stprintf_s(str, _T("%06d"), obj->Mp());
	//GameDevice()->m_pFont->Draw(DestX + 8, DestY + 16, str, 12, RGB(0, 0, 0));
	/*
	int hp = obj->Hp();
	_stprintf_s(str, _T("%d"), hp);
	GameDevice()->m_pFont->Draw(550, 10, str, 24, RGB(0, 255, 0));
	:/
	/*
	VECTOR2 PLPos = obj->Getposition();
	_stprintf_s(str, _T("%f"), PLPos.x);
	GameDevice()->m_pFont->Draw(550, 10, str, 24, RGB(0, 255, 0));
	_stprintf_s(str, _T("%f"), PLPos.y);
	GameDevice()->m_pFont->Draw(550, 30, str, 24, RGB(0, 255, 0));
	*/
	/*
	int score = data->Score();
	_stprintf_s(str, _T("%d"), score);
	GameDevice()->m_pFont->Draw(550, 50, str, 24, RGB(0, 255, 0));
	*/
	/*
	float angle = obj->angleDeg;
	_stprintf_s(str, _T("%f"), angle);
	GameDevice()->m_pFont->Draw(550, 10, str, 24, RGB(0, 255, 0));
	float fangle = obj->finalAngle;
	_stprintf_s(str, _T("%f"), fangle);
	GameDevice()->m_pFont->Draw(550, 30, str, 24, RGB(0, 255, 0));
	*/
}
