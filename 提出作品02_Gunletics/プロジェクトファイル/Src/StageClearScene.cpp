#include "StageClearScene.h"
#include "DataCarrier.h"
#include "AudioManager.h"
#include "Player.h"

StageClearScene::StageClearScene()
{
	DataCarrier* objDc = ObjectManager::FindGameObject<DataCarrier>();
	image = new CSpriteImage("Data/Image/my_scorescene.png");
	sprite = new CSprite;
	isscoreDraw = false;
	istotalDraw = false;
	isRankDraw = false;
	nexttotalDraw = false;
	nextrankDraw = false;
	nextstage = false;
	compscore = false;
	isStart = true;
	score = 0;
	totalscore = 0;

	AudioManager::Audio("Bgm1")->Stop();
 }

StageClearScene::~StageClearScene()
{
	SAFE_DELETE(image);
	SAFE_DELETE(sprite);
}

void StageClearScene::Start()
{
	DataCarrier* objDc = ObjectManager::FindGameObject<DataCarrier>();
	second = objDc->GetSecond();
	tensecond = objDc->GetTenSecond();
	minute = objDc->GetMinute();
	score = 0;
	totalscore = 0;
	isStart = false;
}

void StageClearScene::Update()
{
	if (isStart)
	{
		Start();
	}

	DataCarrier* objDc = ObjectManager::FindGameObject<DataCarrier>();
	CDirectInput* input = GameDevice()->m_pDI;

	if (input->CheckKey(KD_TRG, DIK_RETURN) && !compscore)
	{
		targetscore = objDc->Score();
		isscoreDraw = true;
	}
	if (input->CheckKey(KD_TRG, DIK_RETURN) && nexttotalDraw)
	{
		targetscore = score - ((minute * 60 + tensecond * 10 + second) * 10);
		istotalDraw = true;
	}
	if (input->CheckKey(KD_TRG, DIK_RETURN) && nextrankDraw)
	{
		isRankDraw = true;
	}
	if (input->CheckKey(KD_TRG, DIK_RETURN) && nextstage)
	{
		SceneManager::ChangeScene("PlayScene");
		objDc->ClearScore();
		isStart = true;
	}

	if (isscoreDraw)
	{
		frame++;
		score += frame;
		if (score >= targetscore)
		{
			frame = 0;
			score = targetscore;
			compscore = true;
			nexttotalDraw = true;
			isscoreDraw = false;
		}
	}
	if (istotalDraw)
	{
		frame++;
		totalscore += frame;
		if (totalscore >= targetscore)
		{
			frame = 0;
			totalscore = targetscore;
			nextrankDraw = true;
			istotalDraw = false;
		}
	}
}

void StageClearScene::Draw()
{
	sprite->Draw(image, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	
	DataCarrier* objDc = ObjectManager::FindGameObject<DataCarrier>();
	TCHAR str[256];
	timeposX = 350;
	_stprintf_s(str, _T("%d"), second);
	GameDevice()->m_pFont->Draw(timeposX + 200 + 60 + 40, 175, str, 72, RGB(255, 255, 255));
	_stprintf_s(str, _T("%d"), tensecond);
	GameDevice()->m_pFont->Draw(timeposX + 200 + 60, 175, str, 72, RGB(255, 255, 255));
	_stprintf_s(str, _T("%d"), minute);
	GameDevice()->m_pFont->Draw(timeposX + 190, 175, str, 72, RGB(255, 255, 255));
	GameDevice()->m_pFont->Draw(timeposX + 200 + 27, 170, _T("F"), 72, RGB(255, 255, 255), 1.0f, _T("HGP‘n‰pÌßÚ¾ÞÝ½EB"));

	_stprintf_s(str, _T("%d"), score);
	GameDevice()->m_pFont->Draw(540, 295, str, 72, RGB(255, 255, 255));

	_stprintf_s(str, _T("%d"), totalscore);
	GameDevice()->m_pFont->Draw(WINDOW_WIDTH / 4 + 30, WINDOW_HEIGHT - 240, str, 128, RGB(255, 255, 255));

	if (isRankDraw)
	{
		if (totalscore > 2000)
		{
			GameDevice()->m_pFont->Draw(WINDOW_WIDTH / 10 * 7 - 30, 270, _T("S"), 256, RGB(255, 255, 0), 1.0f, _T("HGP‘n‰pÌßÚ¾ÞÝ½EB"));
		}
		else if (totalscore > 1700)
		{
			GameDevice()->m_pFont->Draw(WINDOW_WIDTH / 10 * 7 - 30, 270, _T("A"), 256, RGB(255, 0, 0), 1.0f, _T("HGP‘n‰pÌßÚ¾ÞÝ½EB"));
		}
		else if (totalscore > 1300)
		{
			GameDevice()->m_pFont->Draw(WINDOW_WIDTH / 10 * 7 - 30, 270, _T("B"), 256, RGB(0, 0, 255), 1.0f, _T("HGP‘n‰pÌßÚ¾ÞÝ½EB"));
		}
		else
		{
			GameDevice()->m_pFont->Draw(WINDOW_WIDTH / 10 * 7 - 30, 270, _T("C"), 256, RGB(0, 255, 255), 1.0f, _T("HGP‘n‰pÌßÚ¾ÞÝ½EB"));
		}
		nextstage = true;
	}
}