#pragma once
#include "SceneBase.h"

/// <summary>
/// ゲームクリヤーシーンのクラス
/// </summary>
class ClearScene : public SceneBase
{
public:
	ClearScene();
	~ClearScene();
	void Start();
	void Update() override;
	void Draw() override;
private:
	CSpriteImage* image;
	CSprite* sprite;
	int timeposX;
	int minute;
	int tensecond;
	int second;
	int targetscore;
	int score;
	int totalscore;
	int frame;
	bool isscoreDraw;
	bool nexttotalDraw;
	bool istotalDraw;
	bool nextrankDraw;
	bool isRankDraw;
	bool nextstage;
	bool compscore;
	bool isStart;
};
