#pragma once
#include "Map.h"

//----------------------------------------------------------------------------
// 一つのマップクラスの定義
//----------------------------------------------------------------------------
struct MapLine;
struct EvtMap;
class  MapPiece
{
public:
	TCHAR                    MapFileName[512]; // マップファイル名    // -- 2019.3.5
	TCHAR                    ImageName[512];   // マップチップイメージファイル名
	CSpriteImage*            MapImage;        // マップチップのスプライトイメージ
	int                      MapX;             // 画面の幅　（マップチップが何個分か）
	int                      MapY;             // 画面の高さ（マップチップが何個分か）
	int                      MapchipWidth;     // 一つのマップチップの幅
	int                      MapchipHeight;    // 一つのマップチップの高さ
	int                      MapLnLength;      // マップライン配列の要素数
	int                      EvtMapLength;     // イベントマップ配列の要素数
	int*                     MapBackTbl;	   // マップ配列　背景
	int*                     MapForeTbl;	   // マップ配列　前景
	MapLine*                 MapLn;		       // マップライン配列
	EvtMap*                  EventMap;		   // イベントマップ配列

public:
	// コンストラクタ
	MapPiece();
	void InitMap();
	bool  MakeERelativeFName(TCHAR* szBaseFullPath, TCHAR* szPath, TCHAR* szERelativeFName);    // -- 2019.3.5

	~MapPiece();
};
