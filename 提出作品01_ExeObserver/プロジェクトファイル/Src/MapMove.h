#pragma once
#include "MapManager.h"

class MapMove : public MapBase {
public:
	MapMove();
	~MapMove();

	void MakeMoveMap(TextReader* txt, int n);
	void Update() override;
	void Draw() override;
	int  checkLimitAndSet(const float minOff, const float maxOff, float& velocityIO, float& upIO);
	bool ActiveOn() { return activeOn; }
	void SetActiveOn(bool on) { activeOn = on; }
	void SetMoveOn(bool on) { moveOn = on; }

private:
	Transform	transformBase;	// 基準位置
	VECTOR3		velocity;		// 基準位置からの増分
	std::string tag;			// タグ名
	int			moveFlag;		// 移動区分    1:平行移動　2:回転　3:拡大縮小
	bool		activeOn;		// 処理・表示ON      false:処理・表示なし　true:処理・表示あり
	bool		moveOn;			// 移動ON      false:移動停止　true:移動実行
	VECTOR3		up;				// 増分
	VECTOR3		minOffset;		// 増分最小値
	VECTOR3		maxOffset;		// 増分最大値
	int			changeFlag;		// 増分値が限界値に到達して方向が入れ替わるとき(出力)　0:移動中　1:方向チェンジ
	int			loop;			// 移動を繰り返すか  0:チェンジの箇所で停止し移動ONを0:移動停止に戻す。 1:移動を繰り返す
};