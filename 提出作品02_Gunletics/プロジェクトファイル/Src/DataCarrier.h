#pragma once
#include "Object2D.h"

/// <summary>
/// ゲームの進行に伴う様々なデータを保存するクラス
/// １．マップ番号
/// ２．スコア値
/// </summary>
class DataCarrier : public Object2D
{
public:
	DataCarrier();
	~DataCarrier();

	void Start() override;
	void Update() override;

	void AddScore(int inScore) { score += inScore; }
	void ClearScore() { score = 0; }
	int  Score() { return score; }
	void SetMapNo(int no) { mapNo = no; }
	int  MapNo() { return mapNo; }
	void Setsecond(int setsec) { second = setsec; };
	void Settensecond(int setten) { tensecond = setten; };
	void Setminute(int setminu) { minute = setminu; };
	int GetSecond() { return second; };
	int GetTenSecond() { return tensecond; };
	int GetMinute() { return minute; };

private:
	int mapNo;
	int score;
	int second;
	int tensecond;
	int minute;
};