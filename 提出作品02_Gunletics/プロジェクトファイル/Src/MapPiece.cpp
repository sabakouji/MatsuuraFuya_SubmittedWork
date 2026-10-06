#include "MapPiece.h"

// コンストラクタ
MapPiece::MapPiece()
{
	MapFileName[0] = _T('\0');    // -- 2019.3.5
	ImageName[0] = _T('\0');
	MapImage = nullptr;
	MapX = 0;
	MapY = 0;
	MapchipWidth = 0;
	MapchipHeight = 0;
	MapLnLength = 0;
	EvtMapLength = 0;
	MapBackTbl = nullptr;
	MapForeTbl = nullptr;
	MapLn = nullptr;
	EventMap = nullptr;
}

//  デストラクタ
MapPiece::~MapPiece()
{
	SAFE_DELETE(MapImage);
	SAFE_DELETE_ARRAY(MapBackTbl);
	SAFE_DELETE_ARRAY(MapForeTbl);
	SAFE_DELETE_ARRAY(MapLn);
	SAFE_DELETE_ARRAY(EventMap);
}

//------------------------------------------------------------------------
//
// MapPieceクラスのマップの初期化
//
//------------------------------------------------------------------------
void MapPiece::InitMap()
{
	int i;

	TCHAR szName[512];

	MakeERelativeFName(MapFileName, ImageName, szName);  // マップファイル名からマップイメージのパスを得る    // -- 2019.3.5

	MapImage = new CSpriteImage(szName);
	MapBackTbl = new int[MapX * MapY];	// 要素数分の配列を確保
	for (i = 0; i < MapX * MapY; i++)
	{
		MapBackTbl[i] = -1;	// -1で初期化
	}
	MapForeTbl = new int[MapX * MapY];	// 要素数分の配列を確保
	for (i = 0; i < MapX * MapY; i++)
	{
		MapForeTbl[i] = -1;	// -1で初期化
	}
}

//-----------------------------------------------------------------------------    // -- 2019.3.5
// 相対(relative)パスから基準フォルダからの相対(ERelative)パスを作成する
//
//   引数      TCHAR*  szBaseFullPath
//             TCHAR*  szPath
//             TCHAR*  szERelativeFName(Out)
//-----------------------------------------------------------------------------
bool    MapPiece::MakeERelativeFName(TCHAR* szBaseFullPath, TCHAR* szPath, TCHAR* szERelativeFName)
{
	const DWORD NUM = 20;

	TCHAR BaseDrive[256], BasePath[512], BaseFName[512], BaseExt[256];
	TCHAR Drive[256], Path[512], FName[512], Ext[256];

	TCHAR* p;
	TCHAR BasePathArray[NUM][256] = { 0 };
	TCHAR PathArray[NUM][256] = { 0 };
	TCHAR* context = nullptr;

	int i, j, n, m, d;

	// ドライブ番号、パス名、ファイル名、拡張子に分解
	_tsplitpath_s(szBaseFullPath, BaseDrive, sizeof(BaseDrive) / sizeof(TCHAR), BasePath, sizeof(BasePath) / sizeof(TCHAR),
		BaseFName, sizeof(BaseFName) / sizeof(TCHAR), BaseExt, sizeof(BaseExt) / sizeof(TCHAR));
	_tsplitpath_s(szPath, Drive, sizeof(Drive) / sizeof(TCHAR), Path, sizeof(Path) / sizeof(TCHAR),
		FName, sizeof(FName) / sizeof(TCHAR), Ext, sizeof(Ext) / sizeof(TCHAR));

	// ドライブ番号があるときは既に絶対パスになっている
	if (Drive[0] != _T('\0'))
	{
		_tcscpy_s(szERelativeFName, 512, szPath);
		return true;
	}

	// パス名を各フォルダ階層に分解
	i = 0;
	p = _tcstok_s(BasePath, _T("/\\"), &context);
	while (p)
	{
		_tcscpy_s(BasePathArray[i], p);
		i++;
		p = _tcstok_s(NULL, _T("/\\"), &context);
	}
	n = i;  // ベースの階層数

	i = 0;
	p = _tcstok_s(Path, _T("/\\"), &context);
	while (p)
	{
		_tcscpy_s(PathArray[i], p);
		i++;
		p = _tcstok_s(NULL, _T("/\\"), &context);
	}

	//	相対パスの'．'の数
	i = 0;
	d = 1;
	if (PathArray[i][0] == _T('.'))
	{
		for (j = 1; PathArray[i][j] != _T('\0'); j++) d++;
		i++;
	}
	else if (PathArray[i][0] == _T('\0'))
	{
		i++;
	}
	else {
		;
	}
	m = i;

	// 基準フォルダからの相対パスの作成
	_tcscpy_s(szERelativeFName, 512, BaseDrive);
	if (BaseDrive[0] != _T('\0')) _tcscat_s(szERelativeFName, 512, _T("/"));
	for (i = 0; i < n - d + 1; i++)
	{
		_tcscat_s(szERelativeFName, 512, BasePathArray[i]);
		_tcscat_s(szERelativeFName, 512, _T("/"));
	}
	for (; PathArray[m][0] != _T('\0'); m++)
	{
		_tcscat_s(szERelativeFName, 512, PathArray[m]);
		_tcscat_s(szERelativeFName, 512, _T("/"));
	}
	_tcscat_s(szERelativeFName, 512, FName);
	_tcscat_s(szERelativeFName, 512, Ext);

	return true;
}

