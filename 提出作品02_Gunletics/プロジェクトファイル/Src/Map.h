#pragma once
#include "Object2D.h"
#include "DataCarrier.h"
#include "MapPiece.h"


//----------------------------------------------------------------------------
// マップ線構造体の定義
// 		始点座標｛Ｘ座標、Ｙ座標｝、終点座標｛Ｘ座標、Ｙ座標｝・・・・
// 		法線座標 (開始位置から終了位置に向かって、左手方向に法線が出来る)
//----------------------------------------------------------------------------
struct  MapLine
{
public:
	VECTOR2 Start;		// 始点座標
	VECTOR2 End;			// 終点座標
	VECTOR2 Normal;		// 法線座標 (開始位置から終了位置に向かって、左手方向に法線が出来る)

public:
	// コンストラクタ
	MapLine()
	{
		Start = VECTOR2(0, 0);
		End = VECTOR2(0, 0);
		Normal = VECTOR2(0, 0);
	}
};
//----------------------------------------------------------------------------
// イベントマップ構造体の定義
// 		座標｛Ｘ座標、Ｙ座標｝、イベントＩＤ、イベントＮｏ(種類を論理和で)、汎用カウンタ
//		イベントＩＤ　1:ＰＣのスタート位置　2:アイテム　3:敵
//		イベントＮｏ　0x01 泉の水    0x02 がまの敵    0x04 オオカミの敵
//					　0x10 救急箱    0x20 扉
//----------------------------------------------------------------------------
struct  EvtMap
{
public:
	VECTOR2     Start;		// 座標
	int         EvtID;		// イベントＩＤ
	DWORD       EvtNo;		// イベントＮｏ
	int         EvtOd;		// イベントオペランド
public:
	// コンストラクタ
	EvtMap()
	{
		Start = VECTOR2(0, 0);
		EvtID = 0;
		EvtNo = 0;
		EvtOd = 0;
	}
	EvtMap(VECTOR2 st, int evID, DWORD evNo, int evOd)
	{
		Start = st;
		EvtID = evID;
		EvtNo = evNo;
		EvtOd = evOd;
	}
};


class MapPiece;
/// <summary>
/// マップを表示する処理
/// </summary>
class Map : public Object2D
{
public:
	Map();
	~Map();

	void Start() override;
	void Update() override;

	bool ReadMap(const TCHAR* FileName, MapPiece*& pMap);
	void ChangeMap(int evtOd);
	void SetMap(int);
	void DrawBack();
	void DrawFore();
	void DrawMapLine();
	void DrawCoord();
	bool isCollisionMoveMap(Object2D* obj, VECTOR2& velocity, MapLine*& pHitmapline);
	bool CheckMapcross(MapLine map, Object2D* obj, VECTOR2& verocity, VECTOR2& hitpos);
	bool CheckLinecross(VECTOR2 a1, VECTOR2 a2, VECTOR2 b1, VECTOR2 b2, VECTOR2& hitpos);
	bool CheckRange(float l, float r, float pt);
	bool CheckMapnear(MapLine MapLn, Object2D* obj, VECTOR2& verocity, VECTOR2& vHitpos);
	float GetDistance(MapLine MapLn, Object2D* obj, VECTOR2& velocity);
	float GetLength(VECTOR2 p1, VECTOR2 p2);
	float GetCross(VECTOR2 a, VECTOR2 b);
	float GetDot(VECTOR2 a, VECTOR2 b);
	VECTOR2 GetCenterPosition(Object2D* obj);

	bool DrawMaplineOK(){ return drawMapLineOn;}
	bool DrawCoordOK(){ return drawCoordOn;}

	bool SearchEvt(int nStart, int nEvtID, DWORD dwEvtNo, int& evOd, VECTOR2& vPos, int& nNext);


private:
	// マップ配列
	MapPiece* mapData;
	// マップ線の描画フラグ
	bool  drawMapLineOn;
	// 座標値の描画フラグ
	bool  drawCoordOn;
	// マップイメージ
	CSpriteImage* backImage;
	// マップスプライト
	CSprite* spriteMap;
	CSprite* spriteBack;
	CSprite* spriteLine;

	DataCarrier* objDc;
};