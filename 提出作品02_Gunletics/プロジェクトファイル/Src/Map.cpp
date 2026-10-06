#include "Map.h"
#include "MapDrawBack.h"
#include "MapDrawFore.h"
#include "Player.h"
#include "EnemyManager.h"
#include "WeaponManager.h"
#include "ItemManager.h"
#include "EffectManager.h"
#include "PlayScene.h"

namespace {
	const float RollSpeed = 0.5f;
	
	const TCHAR MapNameArray[][512] = {
		_T("Data/Script/my_Map1.txt"),
		_T("Data/Script/my_Map2.txt"),
		_T("Data/Script/my_map3.txt"),
		//_T("Data/Script/test_map.txt")
	};
	
};


Map::Map()
{
	SetPriority(100);	   // 一番最初に処理する
	ObjectManager::SetVisible(this, false);   // 自分自身は描画しない

	MapDrawBack* objback = Instantiate<MapDrawBack>();	 // 背景描画オブジェクト生成
	objback->SetParent(this);
	MapDrawFore* objfore = Instantiate<MapDrawFore>();	 // 前景描画オブジェクト生成
	objfore->SetParent(this);

	drawMapLineOn = false;
	drawCoordOn = false;
	mapData = nullptr;
	backImage = new CSpriteImage( _T("Data/image/aozora.png"));
	//backImage = new CSpriteImage( _T("Data/image/hoshizora2.png"));
	spriteMap = new CSprite(); // スプライトオブジェクトの生成
	spriteBack = new CSprite(backImage);
	spriteLine = new CSprite();

	objDc = nullptr;

}

Map::~Map()
{
	SAFE_DELETE(sprite);

	SAFE_DELETE(mapData);
	SAFE_DELETE(spriteMap);
	SAFE_DELETE(spriteBack);
	SAFE_DELETE(spriteLine);
	SAFE_DELETE(backImage);
}

void Map::Start()
{
	objDc = ObjectManager::FindGameObject<DataCarrier>();

	// 開始マップの設定
	SetMap(objDc->MapNo());

}

// ----------------------------------------------------------------------
//
// マップ切り替えの処理　　扉に接触したとき呼ばれる
//
// ----------------------------------------------------------------------
void Map::ChangeMap(int evtOd)
{
	CDirectInput* input = GameDevice()->m_pDI;
	int mapNo = objDc->MapNo();

	// マップチェンジのときは必ず次のマップへ移動する
	// 最後のマップでチェンジするとゲームクリヤーとなる
	if (mapNo < (sizeof(MapNameArray) / (sizeof(TCHAR) * 512) - 1))
	{
		// 次のマップへ行く処理

		// ステージクリヤーを使用しないで次のマップへ行くときはこの処理を行う
		ObjectManager::FindGameObject<EnemyManager>()->DestroyEnemy();
		ObjectManager::FindGameObject<WeaponManager>()->DestroyWeapon();
		ObjectManager::FindGameObject<ItemManager>()->DestroyItem();
		ObjectManager::FindGameObject<EffectManager>()->DestroyEffect();
		SetMap(mapNo + 1);	// 次のマップは

		// 一旦、ステージクリヤーにして、次のマップへ行くときはこの処理を行う
		objDc->SetMapNo(mapNo + 1);
		SceneManager::ChangeScene("StageClearScene");    // ステージクリヤー

	}
	else {
		// ゲームクリヤー
		SceneManager::ChangeScene("ClearScene");    // ゲームクリヤー
	}
	
	
 
	/*
	// マップチェンジのときは、引数のevtOdの値のマップへ移動する
	// 最終マップより大きい値を指定したときはゲームクリヤーとなる
	if (evtOd < (sizeof(MapNameArray) / (sizeof(TCHAR) * 512)))
	{
		// 次のマップへ行く処理

		// ステージクリヤーを使用しないで次のマップへ行くときはこの処理を行う
		//ObjectManager::FindGameObject<EnemyManager>()->DestroyEnemy();
		//ObjectManager::FindGameObject<WeaponManager>()->DestroyWeapon();
		//ObjectManager::FindGameObject<ItemManager>()->DestroyItem();
		//ObjectManager::FindGameObject<EffectManager>()->DestroyEffect();
		//SetMap(evtOd);	// 次のマップは


		// 一旦、ステージクリヤーにして、次のマップへ行くときはこの処理を行う
		objDc->SetMapNo(evtOd);
		SceneManager::ChangeScene("StageClearScene");    // ステージクリヤー

	}
	else {
		// ゲームクリヤー
		SceneManager::ChangeScene("ClearScene");    // ゲームクリヤー
	}
	*/
	
}
// ----------------------------------------------------------------------
//
// 開始マップの設定
//
// 引数　：　int no　マップ番号
//
// ----------------------------------------------------------------------
void Map::SetMap(int no)
{
	// 開始マップＮＯ
	objDc->SetMapNo(no);	// データキャリヤにマップ番号を設定する

	if (mapData != nullptr)  SAFE_DELETE(mapData);

	ReadMap(MapNameArray[objDc->MapNo()], mapData);	  // マップ番号で指定されたマップを読み込みます

	// マップ用スプライトの設定
	spriteMap->SetSrc(mapData->MapImage, 0, 0, mapData->MapchipWidth, mapData->MapchipHeight);

	// ＰＣ開始位置の設定
	VECTOR2 pos = VECTOR2(0, 0);
	for (DWORD i = 0; i < mapData->EvtMapLength; i++)
	{
		if (mapData->EventMap[i].EvtID == 1)  // ＰＣのスタート位置
		{
			pos = mapData->EventMap[i].Start;  // ＰＣのスタート座標
			break;
		}
	}

	// ＰＣ開始位置を設定し、ＨＰを回復する
	Player* obj = ObjectManager::FindGameObject<Player>();
	obj->SetPosition(pos + obj->Transform().center);  // posはチップの左上座標なので中心点を足す
	obj->SetHpforMax();
	
}


// ----------------------------------------------------------------------
//
// マップの更新
//
// ----------------------------------------------------------------------
void  Map::Update()
{
	VECTOR2 vScr;
	Player* objpc = ObjectManager::FindGameObject<Player>();

	// ＰＣの位置からスクロール座標を設定する

	vScr.x = GetCenterPosition(objpc).x - WINDOW_WIDTH / 2;
	if (vScr.x > mapData->MapX * mapData->MapchipWidth - WINDOW_WIDTH)
		vScr.x = mapData->MapX * mapData->MapchipWidth - WINDOW_WIDTH;
	if (vScr.x < 0) vScr.x = 0;

	vScr.y = GetCenterPosition(objpc).y - WINDOW_HEIGHT / 2;
	if (vScr.y > mapData->MapY * mapData->MapchipHeight - WINDOW_HEIGHT)
		vScr.y = mapData->MapY * mapData->MapchipHeight - WINDOW_HEIGHT;
	if (vScr.y < 0) vScr.y = 0;

	GameDevice()->Scroll = vScr;			// スクロール座標を設定

	// マップ線の描画をするかどうか
	if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_P))
	{
		if (drawMapLineOn)
		{
			drawMapLineOn = false;
		}
		else {
			drawMapLineOn = true;
		}
	}

	// 座標値の描画をするかどうか
	if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_O))
	{
		if (drawCoordOn)
		{
			drawCoordOn = false;
		}
		else {
			drawCoordOn = true;
		}
	}

}

// ----------------------------------------------------------------------
//
// イベントマップの探索処理
//
// 引数
// 　　int         Start;		// 探索開始位置
// 　　int         EvtID;		// イベントＩＤ
// 　　DWORD       EvtNo;		// イベントＮｏ
//     int         EvtOd;		// イベントオペランド
// 　　VECTOR2     Pos;			// 座標(OUT)
//     int         Next;       // 次開始位置(OUT) 最終に達したら　-1 
//
// 戻り値
//　　TRUE:見つかった  FALSE:見つからない
//
// ----------------------------------------------------------------------
bool  Map::SearchEvt(int st, int evID, DWORD evNo, int& evOd, VECTOR2& pos, int& nNext)
{
	bool bRet = false;

	if (st < 0 || st >= mapData->EvtMapLength)
	{
		nNext = -1;
		return bRet;
	}

	for (DWORD i = st; i < mapData->EvtMapLength; i++)
	{
		if (mapData->EventMap[i].EvtID == evID &&	    // イベントマップ　ID　敵の出現位置
			mapData->EventMap[i].EvtNo & evNo)			// イベントマップ　NO
		{
			pos = mapData->EventMap[i].Start;	// 発生位置
			evOd = mapData->EventMap[i].EvtOd;	// オペランド
			nNext = i + 1;  // 次の開始位置を設定
			bRet = true;
			break;
		}
	}
	if (!bRet)
	{
		nNext = -1;
	}

	return bRet;
}


// ----------------------------------------------------------------------
//
// マップの背景の描画
// (背景描画クラスMapDrawBackが描画する)
//
// ----------------------------------------------------------------------
void  Map::DrawBack()
{

	// ステージ背景の描画（４方向スクロール）　－－－－－－－－－－
	VECTOR2 scroll = GameDevice()->Scroll;		// スクロール位置
	VECTOR2 scr;

	scr.x = WINDOW_WIDTH - ((int)(scroll.x / 2)) % WINDOW_WIDTH;
	scr.y = WINDOW_HEIGHT - ((int)(scroll.y / 2)) % WINDOW_HEIGHT;

	spriteBack->Draw(backImage, 0, 0, WINDOW_WIDTH - scr.x, WINDOW_HEIGHT - scr.y, scr.x, scr.y);
	spriteBack->Draw(backImage, scr.x, 0, 0, WINDOW_HEIGHT - scr.y, WINDOW_WIDTH - scr.x, scr.y);
	spriteBack->Draw(backImage, 0, scr.y, WINDOW_WIDTH - scr.x, 0, scr.x, WINDOW_HEIGHT - scr.y);
	spriteBack->Draw(backImage, scr.x, scr.y, 0, 0, WINDOW_WIDTH - scr.x, WINDOW_HEIGHT - scr.y);

	// マップ背景の描画　－－－－－－－－－－－－－－－－－－－－－－
	int x, y, no;

	for (y = 0; y < mapData->MapY; y++)
	{
		for (x = 0; x < mapData->MapX; x++)
		{
			no = mapData->MapBackTbl[y * mapData->MapX + x];
			if (no == -1)
			{
				;  		// マップの無い所は描画しない
			}
			else {
				spriteMap->m_ofX = no % 1000 * mapData->MapchipWidth;
				spriteMap->m_ofY = no / 1000 * mapData->MapchipHeight;
				spriteMap->Draw(x * mapData->MapchipWidth - scroll.x, y * mapData->MapchipHeight - scroll.y);
			}
		}
	}

}
// ----------------------------------------------------------------------
//
// マップの前景の描画
// (前景描画クラスMapDrawForeが描画する)
//
// ----------------------------------------------------------------------
void  Map::DrawFore()
{
	int x, y, no;
	VECTOR2 scroll = GameDevice()->Scroll;	   		// スクロール位置

	for (y = 0; y < mapData->MapY; y++)
	{
		for (x = 0; x < mapData->MapX; x++)
		{
			no = mapData->MapForeTbl[y * mapData->MapX + x];
			if (no == -1)
			{
				;		// マップの無い所は描画しない
			}
			else {
				spriteMap->m_ofX = no % 1000 * mapData->MapchipWidth;
				spriteMap->m_ofY = no / 1000 * mapData->MapchipHeight;
				spriteMap->Draw(x * mapData->MapchipWidth - scroll.x, y * mapData->MapchipHeight - scroll.y);
			}
		}
	}

}

// ----------------------------------------------------------------------
//
// マップ線の描画
// (前景描画クラスMapDrawForeが描画する)
//
// ----------------------------------------------------------------------
void  Map::DrawMapLine()
{
	int i;
	VECTOR2 scroll = GameDevice()->Scroll;		  		// スクロール位置

	for (i = 0; i < mapData->MapLnLength; i++)
	{
		spriteLine->DrawLine(mapData->MapLn[i].Start.x - scroll.x, mapData->MapLn[i].Start.y - scroll.y,
			mapData->MapLn[i].End.x - scroll.x, mapData->MapLn[i].End.y - scroll.y, 3, RGB(255, 0, 0));
		float cx = (mapData->MapLn[i].Start.x + mapData->MapLn[i].End.x) / 2;
		float cy = (mapData->MapLn[i].Start.y + mapData->MapLn[i].End.y) / 2;
		float nx = cx + mapData->MapLn[i].Normal.x * 10.0f;
		float ny = cy + mapData->MapLn[i].Normal.y * 10.0f;
		spriteLine->DrawLine(cx - scroll.x, cy - scroll.y, nx - scroll.x, ny - scroll.y, 3, RGB(255, 0, 0));
	}
}
// ----------------------------------------------------------------------
//
// 座標値の描画
// (前景描画クラスMapDrawForeが描画する)
//
// ----------------------------------------------------------------------
void  Map::DrawCoord()
{
	VECTOR2 scroll = GameDevice()->Scroll;		  		// スクロール位置
	Player* pc = ObjectManager::FindGameObject<Player>();
	std::string str;
	str = "スクロール X=" + std::to_string((int)scroll.x) + "   Y=" + std::to_string((int)scroll.y);
	GameDevice()->m_pFont->Draw(350, 10, str.c_str(), 16, RGB(255, 0, 0));
	str = "PC論理座標 X=" + std::to_string((int)pc->Transform().position.x) + "   Y=" + std::to_string((int)pc->Transform().position.y);
	GameDevice()->m_pFont->Draw(600, 10, str.c_str(), 16, RGB(255, 0, 0));
	str = "PC画面座標 X=" + std::to_string((int)pc->Transform().position.x-(int)scroll.x) + "   Y=" + std::to_string((int)pc->Transform().position.y-(int)scroll.y);
	GameDevice()->m_pFont->Draw(850, 10, str.c_str(), 16, RGB(255, 0, 0));
 }

// ----------------------------------------------------------------------------------------
//
// マップの接触判定と適切な位置への移動
//
//   ①　マップ線を突き抜けているかチェックする
//   ②　マップ線に近接しているか（キャラが食い込んでいるか）チェックする
//
//   突き抜けているか近接しているとき、増分値velocityを法線方向に食い込み分だけ、戻してやる
//
//   　　戻り値：突き抜けているか近接しているとき真。pHitmapline:接触したマップ線のアドレスが返る
// ----------------------------------------------------------------------------------------
bool Map::isCollisionMoveMap(Object2D* obj, VECTOR2& velocity, MapLine*& pHitmapline)
{
	int i, n, rw;
	bool bRet = false;
	VECTOR2 vHitpos = VECTOR2(0, 0);
	VECTOR2 vHpw = VECTOR2(0, 0);
	float dist, dw;

	pHitmapline = nullptr;
	dist = 999999;
	for (n = 0, i = 0; i < mapData->MapLnLength; i++) {
		rw = CheckMapcross(mapData->MapLn[i], obj, velocity, vHpw);		// ①　マップ線との突き抜け判定
		if (rw) {	// 突き抜けているとき
			bRet = rw;
			dist = GetDistance(mapData->MapLn[i], obj, velocity);
			n = i;
			vHitpos = vHpw;
			break;
		}
		else {
			rw = CheckMapnear(mapData->MapLn[i], obj, velocity, vHpw);	// ②　マップ線との近接判定（キャラがマップ線に食い込んでいるか）
			if (rw) {
				bRet = rw;
				dw = GetDistance(mapData->MapLn[i], obj, velocity);
				if (dist > dw) {						// 一番近いマップ線を探す
					n = i;
					dist = dw;
					vHitpos = vHpw;
				}
			}
		}
	}

	i = n;
	if (bRet) {	// 突き抜けているか近接しているとき、法線方向に食い込み分だけ、戻してやる

		pHitmapline = &(mapData->MapLn[i]);  // 接触したマップ線のアドレス
		VECTOR2 posUp = velocity + VECTOR2(
			round(-(dist - obj->Sprite()->GetSrcWidth() / 2) * mapData->MapLn[i].Normal.x),	
			round(-(dist - obj->Sprite()->GetSrcHeight() / 2) * mapData->MapLn[i].Normal.y));
		velocity = posUp;
	}

	// 接触しているときのみ２回目のチェックを行う
	if (bRet) {
		bRet = false;	// 一旦クリヤーする
		dist = 999999;
		for (n = 0, i = 0; i < mapData->MapLnLength; i++) {
			rw = CheckMapcross(mapData->MapLn[i], obj, velocity, vHpw);		// マップ線との突き抜け判定
			if (rw) {	// 突き抜けているとき
				bRet = rw;
				dist = GetDistance(mapData->MapLn[i], obj, velocity);
				n = i;
				vHitpos = vHpw;
				break;
			}
			else {
				rw = CheckMapnear(mapData->MapLn[i], obj, velocity, vHpw);	// マップ線との近接判定（キャラがマップ線に食い込んでいるか）
				if (rw) {
					bRet = rw;
					dw = GetDistance(mapData->MapLn[i], obj, velocity);
					if (dist > dw) {						// 一番近いマップ線を探す
						n = i;
						dist = dw;
						vHitpos = vHpw;
					}
				}
			}
		}

		i = n;
		if (bRet) {	// 突き抜けているか近接しているとき、velocityを法線方向に食い込み分だけ、戻してやる

			VECTOR2 posUp = velocity + VECTOR2(
				round(-(dist - obj->Sprite()->GetSrcWidth() / 2) * mapData->MapLn[i].Normal.x),			  // ????????????????????
				round(-(dist - obj->Sprite()->GetSrcHeight() / 2) * mapData->MapLn[i].Normal.y));
			velocity = posUp;
		}
		bRet = true;	// 再度セットする
	}

	return bRet;
}

// ----------------------------------------------------------------------------------------
//
// マップ線との突き抜け判定
//
// 　戻り値：交差しているとき真。交点座標がhitposに返る
//
// ----------------------------------------------------------------------------------------
bool Map::CheckMapcross(MapLine map, Object2D* obj, VECTOR2& verocity, VECTOR2& hitpos)
{

	bool bRet = false;

	// 直線１　ｍ１，ｍ２   ・・・・　マップ線
	VECTOR2  m1 = map.Start, m2 = map.End;
	// 直線２　ｏｊ１，ｏｊ２ ・・・　オブジェクト移動
	VECTOR2  oj1 = GetCenterPosition(obj);
	VECTOR2  oj2 = GetCenterPosition(obj) + verocity;

	// ２直線の交差チェック
	bRet = CheckLinecross(m1, m2, oj1, oj2, hitpos);

	return bRet;
}

// ----------------------------------------------------------------------------------------
//
// ２直線の交差チェック
// 　直線ａと直線ｂの交差チェック。
//
// 　戻り値：交差しているとき真。交点座標がhitposに返る
//
// ----------------------------------------------------------------------------------------
bool Map::CheckLinecross(VECTOR2 a1, VECTOR2 a2, VECTOR2 b1, VECTOR2 b2, VECTOR2& hitpos)
{

	bool bRet = false;

	// 交点　ａｐ
	VECTOR2  ap = VECTOR2(0, 0);
	float d1, d2;

	// 直線の長さが０のとき
	if ((a1.x == a2.x && a1.y == a2.y) ||
		(b1.x == b2.x && b1.x == b2.y)) {
		return bRet;
	}

	// 交点があるか
	float dev = (a2.y - a1.y) * (b2.x - b1.x) - (a2.x - a1.x) * (b2.y - b1.y);
	if (dev == 0) {// 平行線のとき
		return bRet;
	}

	// 交点を求める
	d1 = (b1.y * b2.x - b1.x * b2.y);
	d2 = (a1.y * a2.x - a1.x * a2.y);

	ap.x = d1 * (a2.x - a1.x) - d2 * (b2.x - b1.x);
	ap.x /= dev;
	ap.y = d1 * (a2.y - a1.y) - d2 * (b2.y - b1.y);
	ap.y /= dev;

	// 交点が直線の範囲の中にあるか
	if ((CheckRange(a1.x, a2.x, ap.x) && CheckRange(a1.y, a2.y, ap.y)) &&
		(CheckRange(b1.x, b2.x, ap.x) && CheckRange(b1.y, b2.y, ap.y))) {
		bRet = true;
	}

	if (bRet) {
		hitpos = ap;
	}

	return bRet;
}

// ----------------------------------------------------------------------------------------
//
// 範囲チェック
//
// 　戻り値：ptがｌとｒの間に入っているとき真。
//
// ----------------------------------------------------------------------------------------
bool Map::CheckRange(float l, float r, float pt)
{
	float low, hi;
	float mgn = 0.05f;	// 誤差

	if (l <= r) {
		low = l;
		hi = r;
	}
	else {
		low = r;
		hi = l;
	}
	low -= mgn;
	hi += mgn;

	if (low <= pt && pt <= hi) {
		return true;
	}
	else {
		return false;
	}
}


// ----------------------------------------------------------------------------------------
//
// マップ線との近接チェック
//
// 　キャラがマップ線に食い込んでいるかチェック。
//
// 　戻り値：食い込んでいるとき真。接触位置がvHitposに返る
//
// ----------------------------------------------------------------------------------------
bool Map::CheckMapnear(MapLine MapLn, Object2D* obj, VECTOR2& velocity, VECTOR2& vHitpos)
{
	bool bRet = false;

	// ①　キャラとマップ線の法線方向との食い込みチェックを行う --------------------------------
	// キャラの中心から、マップ線の法線方向にキャラの大きさ分の直線を引く
	VECTOR2  p0, p1, p2;
	p0 = GetCenterPosition( obj) + velocity;
	p1.x = p0.x - MapLn.Normal.x * obj->Sprite()->GetSrcWidth() / 2;
	p1.y = p0.y - MapLn.Normal.y * obj->Sprite()->GetSrcHeight() / 2;
	p2.x = p0.x + MapLn.Normal.x * obj->Sprite()->GetSrcWidth() / 2;
	p2.y = p0.y + MapLn.Normal.y * obj->Sprite()->GetSrcHeight() / 2;

	// 直線とマップ線との交差チェック
	bRet = CheckLinecross(MapLn.Start, MapLn.End, p1, p2, vHitpos);

	// ②　交差していない場合、キャラと垂直下方向との食い込みチェックを行う---------------------
	if (!bRet) {
		// キャラの中心から、垂直下方向にキャラの大きさ分の直線を引く
		p0 = GetCenterPosition(obj) + velocity;

		p1.x = p0.x - 0 * obj->Sprite()->GetSrcWidth() / 2;
		p1.y = p0.y - 1 * obj->Sprite()->GetSrcHeight() / 2;
		p2.x = p0.x + 0 * obj->Sprite()->GetSrcWidth() / 2;
		p2.y = p0.y + 1 * obj->Sprite()->GetSrcHeight() / 2;

		// 直線とマップ線との交差チェック
		bRet = CheckLinecross(MapLn.Start, MapLn.End, p1, p2, vHitpos);
	}
	return bRet;
}

// ----------------------------------------------------------------------------------------
//
// マップ線との距離を求める
//
// 　戻り値： マップ線との距離。ただし、法線方向がプラスとなる。
//
// ----------------------------------------------------------------------------------------
float  Map::GetDistance(MapLine MapLn, Object2D* obj, VECTOR2& velocity)
{
	VECTOR2  b = GetCenterPosition(obj) + velocity - MapLn.Start;
	float len;

	// 内積は、ベクトルの法線方向の距離になる
	// (法線の長さが１のため)
	len = GetDot(MapLn.Normal, b);

	return len;
}

// ----------------------------------------------------------------------------------------
//
// ２点間の距離を求める
//
// ----------------------------------------------------------------------------------------
float Map::GetLength(VECTOR2 p1, VECTOR2 p2)
{
	return sqrtf((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

// ----------------------------------------------------------------------------------------
//
// ベクトルの外積を求める
// （２次元の場合、外積はスカラー値となる）
//
// ----------------------------------------------------------------------------------------
float Map::GetCross(VECTOR2 a, VECTOR2 b)
{
	return  a.x * b.y - a.y * b.x;
}

// ----------------------------------------------------------------------------------------
//
// ベクトルの内積を求める
//
// ----------------------------------------------------------------------------------------
float Map::GetDot(VECTOR2 a, VECTOR2 b)
{
	return  a.x * b.x + a.y * b.y;
}

// ----------------------------------------------------------------------------------------
//
// オブジェクトの中心点の現在位置を求める
//
// ----------------------------------------------------------------------------------------
VECTOR2 Map::GetCenterPosition(Object2D* obj)
{
	return obj->Transform().position - obj->Transform().center
		+ VECTOR2(obj->Sprite()->GetSrcWidth() / 2, obj->Sprite()->GetSrcHeight() / 2);
}

//-----------------------------------------------------------------------------
// マップファイルの読み込み
//
//   引数　　　TCHAR* FName
//             MapPiece* &pMap
//-----------------------------------------------------------------------------
bool Map::ReadMap(const TCHAR* FileName, MapPiece*& pMap)
{
	const int BUFSIZE = 2048;                        // -- 2020.2.15
	FILE* fp;
	TCHAR   szWork[BUFSIZE], ww[BUFSIZE];
	TCHAR* p, * s;
	errno_t error;

	//error = _tfopen_s(&fp, FileName, _T("r, ccs = UNICODE"));	// スクリプト読み込み(Unicodeで読み込み)
	error = _tfopen_s(&fp, FileName, _T("r"));	// スクリプト読み込み
	if (error == 0) {

		SAFE_DELETE(pMap);		// 以前のマップを削除する
		pMap = new MapPiece();		// マップの生成

		_tcscpy_s(pMap->MapFileName, FileName);   // マップ名を保存しておく  // -- 2019.3.5

		while (_fgetts(szWork, BUFSIZE, fp) != NULL)  // ファイルの最後に達するまで一行づつ読み込む
		{
			s = _tcsstr(szWork, _T("\n"));      // 文字列の最後の\nを削除する
			if (s != NULL)
			{
				*s = _T('\0');
			}

			if (_tcsncmp(szWork, _T("@MapData"), 8) == 0)  // マップ全体に関するデータ
			{
				_stscanf_s(szWork, _T("%s %d, %d, %d, %d, %s"), ww, (int)sizeof(ww) / (int)sizeof(TCHAR), &pMap->MapX, &pMap->MapY,
					&pMap->MapchipWidth, &pMap->MapchipHeight, pMap->ImageName, (int)sizeof(pMap->ImageName) / (int)sizeof(TCHAR));
				// マップの初期化
				pMap->InitMap();
			}
			else if (_tcsncmp(szWork, _T("@Back"), 5) == 0)  // 後景マップデータ
			{

				_stscanf_s(szWork, _T("%s"), ww, (int)sizeof(ww) / (int)sizeof(TCHAR));  // タイトル行

				for (int y = 0; y < pMap->MapY; y++)  // マップデータ
				{
					if (_fgetts(szWork, BUFSIZE, fp) == NULL) break;  // 一行読み込む。データがなくなったら終了
					s = _tcsstr(szWork, _T("\n"));      // 文字列の最後の\nを削除する
					if (s != NULL)
					{
						*s = _T('\0');
					}
					p = szWork;
					for (int x = 0; x < pMap->MapX; x++)
					{
						_stscanf_s(p, _T("%d,"), &pMap->MapBackTbl[y * pMap->MapX + x]);
						p = _tcschr(p, _T(','));
						p++;
					}
				}
			}
			else if (_tcsncmp(szWork, _T("@Fore"), 5) == 0) {  // 前景マップデータ

				_stscanf_s(szWork, _T("%s"), ww, (int)sizeof(ww) / (int)sizeof(TCHAR));  // タイトル行

				for (int y = 0; y < pMap->MapY; y++)  // マップデータ
				{
					if (_fgetts(szWork, BUFSIZE, fp) == NULL) break;  // 一行読み込む。データがなくなったら終了
					s = _tcsstr(szWork, _T("\n"));      // 文字列の最後の\nを削除する
					if (s != NULL)
					{
						*s = _T('\0');
					}
					p = szWork;
					for (int x = 0; x < pMap->MapX; x++)
					{
						_stscanf_s(p, _T("%d,"), &pMap->MapForeTbl[y * pMap->MapX + x]);
						p = _tcschr(p, _T(','));
						p++;
					}
				}
			}
			else if (_tcsncmp(szWork, _T("@MapLine"), 8) == 0) {  // マップ線データ

				_stscanf_s(szWork, _T("%s %d"), ww, (int)sizeof(ww) / (int)sizeof(TCHAR), &pMap->MapLnLength);  // タイトル行

				pMap->MapLn = new MapLine[pMap->MapLnLength];

				for (int i = 0; i < pMap->MapLnLength; i++)  // マップ線データ
				{
					if (_fgetts(szWork, BUFSIZE, fp) == NULL) break;  // 一行読み込む。データがなくなったら終了
					s = _tcsstr(szWork, _T("\n"));      // 文字列の最後の\nを削除する
					if (s != NULL)
					{
						*s = _T('\0');
					}
					p = szWork;
					_stscanf_s(p, _T("%f,"), &pMap->MapLn[i].Start.x);
					p = _tcschr(p, _T(','));
					p++;
					_stscanf_s(p, _T("%f,"), &pMap->MapLn[i].Start.y);
					p = _tcschr(p, _T(','));
					p++;
					_stscanf_s(p, _T("%f,"), &pMap->MapLn[i].End.x);
					p = _tcschr(p, _T(','));
					p++;
					_stscanf_s(p, _T("%f,"), &pMap->MapLn[i].End.y);
					p = _tcschr(p, _T(','));
					p++;
					_stscanf_s(p, _T("%f,"), &pMap->MapLn[i].Normal.x);
					p = _tcschr(p, _T(','));
					p++;
					_stscanf_s(p, _T("%f,"), &pMap->MapLn[i].Normal.y);
					p = _tcschr(p, _T(','));
					p++;
				}
			}

			else if (_tcsncmp(szWork, _T("@EvtMap"), 7) == 0) {  // イベントマップデータ

				_stscanf_s(szWork, _T("%s %d"), ww, (int)sizeof(ww) / (int)sizeof(TCHAR), &pMap->EvtMapLength);  // タイトル行
				pMap->EventMap = new EvtMap[pMap->EvtMapLength];

				for (int i = 0; i < pMap->EvtMapLength; i++)  // イベントマップデータ
				{
					if (_fgetts(szWork, BUFSIZE, fp) == NULL) break;  // 一行読み込む。データがなくなったら終了
					s = _tcsstr(szWork, _T("\n"));      // 文字列の最後の\nを削除する
					if (s != NULL)
					{
						*s = _T('\0');
					}
					p = szWork;

					_stscanf_s(p, _T("%f,"), &pMap->EventMap[i].Start.x);
					p = _tcschr(p, _T(','));
					p++;
					_stscanf_s(p, _T("%f,"), &pMap->EventMap[i].Start.y);
					p = _tcschr(p, _T(','));
					p++;

					_stscanf_s(p, _T("%d,"), &pMap->EventMap[i].EvtID);
					p = _tcschr(p, _T(','));
					p++;
					_stscanf_s(p, _T("%x,"), &pMap->EventMap[i].EvtNo);
					p = _tcschr(p, _T(','));
					p++;
					_stscanf_s(p, _T("%d,"), &pMap->EventMap[i].EvtOd);
					p = _tcschr(p, _T(','));
					p++;
				}
			}
			else {
				;  // タイトルのない行は読み飛ばす
			}
		}
		fclose(fp);
	}
	return true;
}



