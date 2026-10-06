// ========================================================================================
//
// シェーダーの処理                                               ver 3.0        2021.1.11
//
//   シェーダーグラムの読み込みとシェーダーの作成
//   インプットレイアウトの作成
//   コンスタントバッファの作成
//
//
//    登録されているシェーダー
//
//		・シンプルなシェーダー                          InitShaderSimple();
//		・スプライト用のシェーダー                      InitShaderSprite();
//		・FBXStaticMesh/FBXSkinMesh用のシェーダー       InitShaderFbx();
//		・ディスプレイスメントマッピング用のシェーダー  InitShaderDisplace();
//		・エフェクト用のシェーダー                      InitShaderEffect();
//
//                                                                              Shader.cpp
// ========================================================================================

#include "Shader.h"
#include "FbxMesh.h"

//------------------------------------------------------------------------
//
//	シェーダーのコンストラクタ	
//
//  引数　CDirect3D* pD3D
//
//------------------------------------------------------------------------
CShader::CShader(CDirect3D* pD3D)
{
	ZeroMemory(this, sizeof(CShader));
	m_pD3D = pD3D;
}
//------------------------------------------------------------------------
//
//	シェーダーのデストラクタ	
//
//------------------------------------------------------------------------
CShader::~CShader()
{
	// シェーダーの解放  ------------------------------------------

	// ComPtr のため解放は自動（デストラクタ）。明示的に Reset で前倒し解放する。
	// 通常用のシンプルなシェーダー
	m_pSimple_VertexLayout.Reset();
	m_pSimple_VS.Reset();
	m_pSimple_PS.Reset();

	// 3Dスプライト用のシェーダー
	m_pSprite3D_VertexLayout.Reset();
	m_pSprite3D_VS.Reset();
	m_pSprite3D_PS.Reset();
	m_pSprite3D_VS_BILL.Reset();

	// ディスプレースメントマッピング(波)用のシェーダー
	m_pDisplaceWave_VertexLayout.Reset();
	m_pDisplaceWave_VS.Reset();
	m_pDisplaceWave_HS.Reset();
	m_pDisplaceWave_DS.Reset();
	m_pDisplaceWave_PS.Reset();

	// ディスプレースメントマッピング(メッシュ)用のシェーダー
	m_pDisplaceSkinMesh_VS.Reset();
	m_pDisplaceSkinMesh_HS.Reset();
	m_pDisplaceSkinMesh_DS.Reset();
	m_pDisplaceSkinMesh_PS.Reset();

	m_pDisplaceStaticMesh_VS.Reset();
	m_pDisplaceStaticMesh_HS.Reset();
	m_pDisplaceStaticMesh_DS.Reset();
	m_pDisplaceStaticMesh_PS.Reset();

	// エフェクト用のシェーダー
	m_pEffect3D_VertexLayout.Reset();
	m_pEffect3D_VS_POINT.Reset();
	m_pEffect3D_GS_POINT.Reset();
	m_pEffect3D_PS.Reset();
	m_pEffect3D_VertexLayout_BILL.Reset();
	m_pEffect3D_VS_BILL.Reset();
	m_pEffect3D_VS_BILLMESH.Reset();              // -- 2019.7.17

	// Fbxモデル　スタティツクメッシュ用のシェーダー
	// （Normalマッピング）
	m_pFbxStaticMesh_VertexLayout.Reset();
	m_pFbxStaticMesh_VS.Reset();
	m_pFbxStaticMesh_PS.Reset();

	// Fbxモデル　スキンメッシュ用のシェーダー
	// （Normalマッピング）
	m_pFbxSkinMesh_VertexLayout.Reset();
	m_pFbxSkinMesh_VS.Reset();
	m_pFbxSkinMesh_PS.Reset();

	// トゥーンシェーディング用のシェーダー
	m_pToon_VertexLayout.Reset();
	m_pToon_VS.Reset();
	m_pToonSkin_VertexLayout.Reset();
	m_pToonSkin_VS.Reset();
	m_pToon_PS.Reset();

	// 輪郭線用のシェーダー
	m_pOutline_VertexLayout.Reset();
	m_pOutline_VS.Reset();
	m_pOutlineSkin_VertexLayout.Reset();
	m_pOutlineSkin_VS.Reset();
	m_pOutline_PS.Reset();


	// コンスタントバッファの解放 ---------------------------

	m_pConstantBufferDisplace.Reset();
	m_pConstantBufferEffect.Reset();
	m_pConstantBufferSprite3D.Reset();

	m_pConstantBufferWVLED.Reset();
	m_pConstantBufferBone2.Reset();
	m_pConstantBufferMaterial.Reset();      // -- 2020.12.15
	m_pConstantBufferOutline.Reset();
	m_pConstantBufferToon.Reset();

}

//------------------------------------------------------------------------
//
//	各種シェーダーの作成	
//
//  ・シェーダーとコンスタントバッファを作成する
//  ・テクスチャーサンプラーとブレンドステートを作成する
//
//  引数　なし
//
//	戻り値 HRESULT
//		S_OK	= 正常
//		E_FAIL	= 異常
//
//------------------------------------------------------------------------
HRESULT CShader::InitShader()
{
	//  通常メッシュ用のシェーダー作成
	InitShaderSimple();

	//  Fbxスタティック・スキンメッシュ用のシェーダー作成
	InitShaderFbx();

	//  ディスプレースメントマッピング用のシェーダー作成
	InitShaderDisplace();

	//  エフェクト用のシェーダー作成
	InitShaderEffect();

	//  スプライト用のシェーダー作成
	InitShaderSprite();

	//  トゥーンシェーディング用のシェーダー作成
	InitShaderToon();

	//  輪郭線用のシェーダー作成
	InitShaderOutline();

	//  コンスタントバッファ作成
	InitShaderConstant();

	return S_OK;
}
//------------------------------------------------------------------------
//
//  通常用(Simple Shader)のシェーダー作成
//
//  引数　なし
//
//	戻り値 HRESULT
//		S_OK	= 正常
//		E_FAIL	= 異常
//
//------------------------------------------------------------------------
HRESULT CShader::InitShaderSimple()
{

	// 頂点インプットレイアウトを定義
	UINT numElements = 0;
	D3D11_INPUT_ELEMENT_DESC Simplelayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	numElements = sizeof(Simplelayout) / sizeof(Simplelayout[0]);

	// バーテックスシェーダ・ピクセルシェーダ・頂点インプットレイアウトの作成
	MakeShader(_T("VS"), _T("Simple_VS.cso"), (void**)&m_pSimple_VS, Simplelayout, numElements, &m_pSimple_VertexLayout);
	MakeShader(_T("PS"), _T("Simple_PS.cso"), (void**)&m_pSimple_PS);

	return S_OK;
}

//------------------------------------------------------------------------
//
//  Fbxモデル　スタティツク＆スキンメッシュ用のシェーダー作成
//
//  引数　なし
//
//	戻り値 HRESULT
//		S_OK	= 正常
//		E_FAIL	= 異常
//
//------------------------------------------------------------------------
HRESULT CShader::InitShaderFbx()
{

	// -------------------------------------------------------------------
	// 
	// スタティックメッシュ  FbxStaticMesh
	// 
	// -------------------------------------------------------------------
	// 頂点インプットレイアウトを定義
	UINT numElements = 0;
	D3D11_INPUT_ELEMENT_DESC FbxStaticNM_layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },		// 計32byte
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,32, D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,44, D3D11_INPUT_PER_VERTEX_DATA,0 }, // 計56byte
	};
	numElements = sizeof(FbxStaticNM_layout) / sizeof(FbxStaticNM_layout[0]);

	// バーテックスシェーダ・ピクセルシェーダ・頂点インプットレイアウトの作成
	MakeShader(_T("VS"), _T("FbxStaticMesh_VS.cso"), (void**)&m_pFbxStaticMesh_VS, FbxStaticNM_layout, numElements, &m_pFbxStaticMesh_VertexLayout);
	MakeShader(_T("PS"), _T("FbxStaticMesh_PS.cso"), (void**)&m_pFbxStaticMesh_PS);


	// -------------------------------------------------------------------
	// 
	// スキンメッシュ  FbxSkinMesh
	// 
	// -------------------------------------------------------------------
	// 頂点インプットレイアウトを定義
	numElements = 0;
	D3D11_INPUT_ELEMENT_DESC FbxSkinNM_layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_INDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 76, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // 計76Byte
	};
	numElements = sizeof(FbxSkinNM_layout) / sizeof(FbxSkinNM_layout[0]);

	// バーテックスシェーダ・ピクセルシェーダ・頂点インプットレイアウトの作成
	MakeShader(_T("VS"), _T("FbxSkinMesh_VS.cso"), (void**)&m_pFbxSkinMesh_VS, FbxSkinNM_layout, numElements, &m_pFbxSkinMesh_VertexLayout);
	MakeShader(_T("PS"), _T("FbxSkinMesh_PS.cso"), (void**)&m_pFbxSkinMesh_PS);

	return S_OK;
}
//------------------------------------------------------------------------
//
//  トゥーンシェーディング用のシェーダー作成
//
//  スタティックメッシュ用(Toon_VS)・スキンメッシュ用(ToonSkin_VS)と
//  両者共通のピクセルシェーダー(Toon_PS)を作成する。
//  頂点バッファは FbxStaticMesh / FbxSkinMesh と同じものを使うため、
//  インプットレイアウトはオフセットを合わせつつ必要な要素のみ宣言する。
//
//  引数　なし
//
//	戻り値 HRESULT
//		S_OK	= 正常
//		E_FAIL	= 異常
//
//------------------------------------------------------------------------
HRESULT CShader::InitShaderToon()
{
	// -------------------------------------------------------------------
	// スタティックメッシュ用（頂点バッファは StaticVertexNormal）
	// -------------------------------------------------------------------
	D3D11_INPUT_ELEMENT_DESC ToonStatic_layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(ToonStatic_layout) / sizeof(ToonStatic_layout[0]);

	MakeShader(_T("VS"), _T("Toon_VS.cso"), (void**)&m_pToon_VS, ToonStatic_layout, numElements, &m_pToon_VertexLayout);

	// -------------------------------------------------------------------
	// スキンメッシュ用（頂点バッファは SkinVertexNormal）
	// -------------------------------------------------------------------
	D3D11_INPUT_ELEMENT_DESC ToonSkin_layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_INDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	numElements = sizeof(ToonSkin_layout) / sizeof(ToonSkin_layout[0]);

	MakeShader(_T("VS"), _T("ToonSkin_VS.cso"), (void**)&m_pToonSkin_VS, ToonSkin_layout, numElements, &m_pToonSkin_VertexLayout);

	// スタティック／スキン共通のピクセルシェーダー
	MakeShader(_T("PS"), _T("Toon_PS.cso"), (void**)&m_pToon_PS);

	return S_OK;
}
//------------------------------------------------------------------------
//
//  輪郭線用のシェーダー作成
//
//  法線方向へ押し出したシェルを描画する。前面カリング
//  (CDirect3D::m_pRStateFrontCull)と組み合わせて使用すること。
//
//  引数　なし
//
//	戻り値 HRESULT
//		S_OK	= 正常
//		E_FAIL	= 異常
//
//------------------------------------------------------------------------
HRESULT CShader::InitShaderOutline()
{
	// -------------------------------------------------------------------
	// スタティックメッシュ用（頂点バッファは StaticVertexNormal）
	// -------------------------------------------------------------------
	D3D11_INPUT_ELEMENT_DESC OutlineStatic_layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(OutlineStatic_layout) / sizeof(OutlineStatic_layout[0]);

	MakeShader(_T("VS"), _T("Outline_VS.cso"), (void**)&m_pOutline_VS, OutlineStatic_layout, numElements, &m_pOutline_VertexLayout);

	// -------------------------------------------------------------------
	// スキンメッシュ用（頂点バッファは SkinVertexNormal）
	// -------------------------------------------------------------------
	D3D11_INPUT_ELEMENT_DESC OutlineSkin_layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_INDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	numElements = sizeof(OutlineSkin_layout) / sizeof(OutlineSkin_layout[0]);

	MakeShader(_T("VS"), _T("OutlineSkin_VS.cso"), (void**)&m_pOutlineSkin_VS, OutlineSkin_layout, numElements, &m_pOutlineSkin_VertexLayout);

	// スタティック／スキン共通のピクセルシェーダー
	MakeShader(_T("PS"), _T("Outline_PS.cso"), (void**)&m_pOutline_PS);

	return S_OK;
}
//------------------------------------------------------------------------
//
//  ディスプレースメントマッピング用のシェーダー作成
//
//  引数　なし
//
//	戻り値 HRESULT
//		S_OK	= 正常
//		E_FAIL	= 異常
//
//------------------------------------------------------------------------
HRESULT CShader::InitShaderDisplace()
{

	// -----------------------------------------------------------------------------------------------
	// 波のディスプレイスマッピング DisplaceWave
	// -----------------------------------------------------------------------------------------------
	// 頂点インプットレイアウトを定義
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	// バーテックスシェーダ・ピクセルシェーダ・頂点インプットレイアウトの作成
	MakeShader(_T("VS"), _T("DisplaceWave_VS.cso"), (void**)&m_pDisplaceWave_VS, layout, numElements, &m_pDisplaceWave_VertexLayout);
	MakeShader(_T("HS"), _T("DisplaceWave_HS.cso"), (void**)&m_pDisplaceWave_HS);
	MakeShader(_T("DS"), _T("DisplaceWave_DS.cso"), (void**)&m_pDisplaceWave_DS);
	MakeShader(_T("PS"), _T("DisplaceWave_PS.cso"), (void**)&m_pDisplaceWave_PS);


	// -----------------------------------------------------------------------------------------------
	// スタティックメッシュのディスプレイスメントマッピング DisplaceStaticMesh
	// -----------------------------------------------------------------------------------------------
	// バーテックスシェーダ・ピクセルシェーダ・ハルシェーダ・ドメインシェーダの作成
	//  !!!!! 頂点インプットレイアウトは、スタティックメッシュのレイアウトを使用	
	MakeShader(_T("VS"), _T("DisplaceStaticMesh_VS.cso"), (void**)&m_pDisplaceStaticMesh_VS);
	MakeShader(_T("HS"), _T("DisplaceStaticMesh_HS.cso"), (void**)&m_pDisplaceStaticMesh_HS);
	MakeShader(_T("DS"), _T("DisplaceStaticMesh_DS.cso"), (void**)&m_pDisplaceStaticMesh_DS);
	MakeShader(_T("PS"), _T("DisplaceStaticMesh_PS.cso"), (void**)&m_pDisplaceStaticMesh_PS);

	// -----------------------------------------------------------------------------------------------
	// スキンメッシュのディスプレイスメントマッピング DisplaceSkinMesh
	// -----------------------------------------------------------------------------------------------
	// バーテックスシェーダ・ピクセルシェーダ・ハルシェーダ・ドメインシェーダの作成
	//  !!!!! 頂点インプットレイアウトは、スキンメッシュのレイアウトを使用	
	MakeShader(_T("VS"), _T("DisplaceSkinMesh_VS.cso"), (void**)&m_pDisplaceSkinMesh_VS);
	MakeShader(_T("HS"), _T("DisplaceSkinMesh_HS.cso"), (void**)&m_pDisplaceSkinMesh_HS);
	MakeShader(_T("DS"), _T("DisplaceSkinMesh_DS.cso"), (void**)&m_pDisplaceSkinMesh_DS);
	MakeShader(_T("PS"), _T("DisplaceSkinMesh_PS.cso"), (void**)&m_pDisplaceSkinMesh_PS);


	return S_OK;
}

//------------------------------------------------------------------------
//
//  エフェクト用のシェーダー作成
//
//  引数　なし
//
//	戻り値 HRESULT
//		S_OK	= 正常
//		E_FAIL	= 異常
//
//------------------------------------------------------------------------
HRESULT CShader::InitShaderEffect()
{

	// -------------------------------------------------------------------
	// 
	// パーティクルのシェーダー
	// 
	// -------------------------------------------------------------------
	// 頂点インプットレイアウトを定義
	UINT numElements = 0;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	numElements = sizeof(layout) / sizeof(layout[0]);

	// バーテックスシェーダ・ジオメトリシェーダ・頂点インプットレイアウトの作成
	MakeShader(_T("VS"), _T("Effect3D_VS_POINT.cso"), (void**)&m_pEffect3D_VS_POINT, layout, numElements, &m_pEffect3D_VertexLayout);
	MakeShader(_T("GS"), _T("Effect3D_GS_POINT.cso"), (void**)&m_pEffect3D_GS_POINT);


	// -----------------------------------------------------------------------------------------------------
	// 
	// ビルボードのシェーダー
	// 
	// -----------------------------------------------------------------------------------------------------
	//頂点インプットレイアウトを定義
	D3D11_INPUT_ELEMENT_DESC layoutbill[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	numElements = sizeof(layoutbill) / sizeof(layoutbill[0]);

	// バーテックスシェーダ・頂点インプットレイアウトの作成
	MakeShader(_T("VS"), _T("Effect3D_VS_BILL.cso"), (void**)&m_pEffect3D_VS_BILL, layoutbill, numElements, &m_pEffect3D_VertexLayout_BILL);

	// -----------------------------------------------------------------------------------------------------
	// 
	// ビルボードメッシュのシェーダー
	// 
	// -----------------------------------------------------------------------------------------------------
	// バーテックスシェーダの作成
	MakeShader(_T("VS"), _T("Effect3D_VS_BILLMESH.cso"), (void**)&m_pEffect3D_VS_BILLMESH);

	// -----------------------------------------------------------------------------------------------------
	// 
	// 共通のシェーダー
	// 
	// -----------------------------------------------------------------------------------------------------
	// ピクセルシェーダの作成
	MakeShader(_T("PS"), _T("Effect3D_PS.cso"), (void**)&m_pEffect3D_PS);

	return S_OK;
}

//------------------------------------------------------------------------
//
//  スプライト用のシェーダー作成
//
//  引数　なし
//
//	戻り値 HRESULT
//		S_OK	= 正常
//		E_FAIL	= 異常
//
//------------------------------------------------------------------------
HRESULT CShader::InitShaderSprite()
{
	// 頂点インプットレイアウトを定義	
	D3D11_INPUT_ELEMENT_DESC layout_sprite[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout_sprite) / sizeof(layout_sprite[0]);

	// バーテックスシェーダ・ピクセルシェーダ・頂点インプットレイアウトの作成
	MakeShader(_T("VS"), _T("Sprite3D_VS.cso"), (void**)&m_pSprite3D_VS, layout_sprite, numElements, &m_pSprite3D_VertexLayout);
	MakeShader(_T("VS"), _T("Sprite3D_VS_BILL.cso"), (void**)&m_pSprite3D_VS_BILL);
	MakeShader(_T("PS"), _T("Sprite3D_PS.cso"), (void**)&m_pSprite3D_PS);

	return S_OK;
}

//------------------------------------------------------------------------
//
//  シェーダーの作成関数
//
//  引数	TCHAR ProfileName[]	作成するシェーダー種類
//								(VS,PS,GS,HS,DS,CS)
//			TCHAR FileName[]	ＨＬＳＬファイル名
//			void** ppShader		作成するシェーダー(OUT)
//			D3D11_INPUT_ELEMENT_DESC Fluid_layout[]	頂点レイアウト定義(省略可)
//			UINT numElements						頂点レイアウトエレメント数(省略可)
//			ID3D11InputLayout** ppInputLayout		作成する頂点レイアウト(OUT)(省略可)
//
//	戻り値 HRESULT
//		S_OK	= 正常
//		E_FAIL	= 異常
//
//------------------------------------------------------------------------
HRESULT CShader::MakeShader(const TCHAR ProfileName[], const TCHAR FileName[], void** ppShader, D3D11_INPUT_ELEMENT_DESC Fluid_layout[], UINT numElements, ID3D11InputLayout** ppInputLayout)
{
	// コンパイル済みシェーダの読み込み配列
	BYTE* pCso = nullptr;
	DWORD dwCsoSize = 0;

	// コンパイル済みシェーダの読み込みをする
	m_pD3D->ReadCso(FileName, &pCso, &dwCsoSize);

	// シェーダー種類ごとの作成処理
	if (_tcscmp(ProfileName, _T("VS")) == 0)	// バーテックスシェーダー
	{
		if (FAILED(m_pD3D->m_pDevice->CreateVertexShader(pCso, dwCsoSize, nullptr, (ID3D11VertexShader**)ppShader)))
		{
			SAFE_DELETE_ARRAY(pCso);
			MessageBox(0, _T("バーテックスシェーダー作成失敗"), FileName, MB_OK);
			return E_FAIL;
		}
		if (ppInputLayout)	// 頂点インプットレイアウトを作成するとき
		{
			// 頂点インプットレイアウトを作成
			if (FAILED(m_pD3D->m_pDevice->CreateInputLayout(Fluid_layout, numElements, pCso, dwCsoSize, ppInputLayout)))
			{
				MessageBox(0, _T("インプット レイアウト作成失敗"), FileName, MB_OK);
				return E_FAIL;
			}
		}
	}
	else if (_tcscmp(ProfileName, _T("PS")) == 0)	// ピクセルシェーダー
	{
		if (FAILED(m_pD3D->m_pDevice->CreatePixelShader(pCso, dwCsoSize, nullptr, (ID3D11PixelShader**)ppShader)))
		{
			SAFE_DELETE_ARRAY(pCso);
			MessageBox(0, _T("ピクセルシェーダー作成失敗"), FileName, MB_OK);
			return E_FAIL;
		}
	}
	else if (_tcscmp(ProfileName, _T("GS")) == 0)	// ジオメトリシェーダー
	{
		if (FAILED(m_pD3D->m_pDevice->CreateGeometryShader(pCso, dwCsoSize, nullptr, (ID3D11GeometryShader**)ppShader)))
		{
			SAFE_DELETE_ARRAY(pCso);
			MessageBox(0, _T("ジオメトリシェーダー作成失敗"), FileName, MB_OK);
			return E_FAIL;
		}
	}
	else if (_tcscmp(ProfileName, _T("HS")) == 0)	// ハルシェーダー
	{
		if (FAILED(m_pD3D->m_pDevice->CreateHullShader(pCso, dwCsoSize, nullptr, (ID3D11HullShader**)ppShader)))
		{
			SAFE_DELETE_ARRAY(pCso);
			MessageBox(0, _T("ハルシェーダー作成失敗"), FileName, MB_OK);
			return E_FAIL;
		}
	}
	else if (_tcscmp(ProfileName, _T("DS")) == 0)	// ドメインシェーダー
	{
		if (FAILED(m_pD3D->m_pDevice->CreateDomainShader(pCso, dwCsoSize, nullptr, (ID3D11DomainShader**)ppShader)))
		{
			SAFE_DELETE_ARRAY(pCso);
			MessageBox(0, _T("ドメインシェーダー作成失敗"), FileName, MB_OK);
			return E_FAIL;
		}
	}
	else if (_tcscmp(ProfileName, _T("CS")) == 0)	// コンピュートシェーダ
	{
		if (FAILED(m_pD3D->m_pDevice->CreateComputeShader(pCso, dwCsoSize, nullptr, (ID3D11ComputeShader**)ppShader)))
		{
			SAFE_DELETE_ARRAY(pCso);
			MessageBox(0, _T("コンピュートシェーダ作成失敗"), FileName, MB_OK);
			return E_FAIL;
		}
	}
	else {
		SAFE_DELETE_ARRAY(pCso);
		MessageBox(0, _T("シェーダ種類指定エラー"), ProfileName, MB_OK);
		return E_FAIL;
	}

	SAFE_DELETE_ARRAY(pCso);
	return S_OK;

}

//------------------------------------------------------------------------
//
//  各種コンスタントバッファー作成
//
//  引数　なし
//
//	戻り値 HRESULT
//		S_OK	= 正常
//		E_FAIL	= 異常
//
//------------------------------------------------------------------------
HRESULT CShader::InitShaderConstant()
{

	// ディスプレイスメントマッピング用 コンスタントバッファー作成
	MakeConstantBuffer(sizeof(CONSTANT_BUFFER_DISPLACE), &m_pConstantBufferDisplace);

	// エフェクト用 コンスタントバッファー作成
	MakeConstantBuffer(sizeof(CONSTANT_BUFFER_EFFECT), &m_pConstantBufferEffect);

	// スプライト用 コンスタントバッファー作成　ここでは変換行列渡し用
	MakeConstantBuffer(sizeof(CONSTANT_BUFFER_SPRITE), &m_pConstantBufferSprite3D);

	// Fbxコンスタントバッファー作成　ここでは変換行列渡し用
	MakeConstantBuffer(sizeof(CONSTANT_BUFFER_WVLED), &m_pConstantBufferWVLED);

	// Fbxコンスタントバッファー作成　ここではボーン行列渡し用
	MakeConstantBuffer(sizeof(MATRIX4X4) * MAX_BONES, &m_pConstantBufferBone2);

	// コンスタントバッファー作成　マテリアル渡し用                                    // -- 2020.12.15
	MakeConstantBuffer(sizeof(CONSTANT_BUFFER_MATERIAL), &m_pConstantBufferMaterial);

	// コンスタントバッファー作成　輪郭線パラメータ渡し用
	MakeConstantBuffer(sizeof(CONSTANT_BUFFER_OUTLINE), &m_pConstantBufferOutline);

	// コンスタントバッファー作成　トゥーンパラメータ渡し用
	MakeConstantBuffer(sizeof(CONSTANT_BUFFER_TOON), &m_pConstantBufferToon);

	return S_OK;
}

//------------------------------------------------------------------------
//
//  コンスタントバッファーの作成関数
//
//  引数	UINT	size						作成するコンスタントバッファーのサイズ
//			ID3D11Buffer**  pppConstantBuffer	作成するコンスタントバッファー(OUT)
//
//	戻り値 HRESULT
//		S_OK	= 正常
//		E_FAIL	= 異常
//
//------------------------------------------------------------------------
HRESULT CShader::MakeConstantBuffer(UINT size, ID3D11Buffer**  ppConstantBuffer)
{
	D3D11_BUFFER_DESC cb = { 0 };

	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = size;
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(m_pD3D->m_pDevice->CreateBuffer(&cb, nullptr, ppConstantBuffer)))
	{
		MessageBox(0, _T("コンスタントバッファー 作成失敗"), nullptr, MB_OK);
		return E_FAIL;
	}
	return S_OK;
}

