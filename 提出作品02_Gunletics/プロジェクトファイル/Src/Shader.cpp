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
//		・スプライト用のシェーダー                      InitShaderSprite();
//
//                                                                              Shader.cpp
// ========================================================================================

#include "Shader.h"

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

	// 2Dスプライト用のシェーダー
	SAFE_RELEASE(m_pSprite2D_VertexLayout);
	SAFE_RELEASE(m_pSprite2D_VS);
	SAFE_RELEASE(m_pSprite2D_PS);

	// コンスタントバッファの解放 ---------------------------
	SAFE_RELEASE(m_pConstantBufferSprite2D);

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

	//  スプライト用のシェーダー作成
	InitShaderSprite();

	//  コンスタントバッファ作成
	InitShaderConstant();

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
	MakeShader(_T("VS"), _T("Sprite2D_VS.cso"), (void**)&m_pSprite2D_VS, layout_sprite, numElements, &m_pSprite2D_VertexLayout);
	MakeShader(_T("PS"), _T("Sprite2D_PS.cso"), (void**)&m_pSprite2D_PS);

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
	// スプライト用 コンスタントバッファー作成　ここでは変換行列渡し用
	MakeConstantBuffer(sizeof(CONSTANT_BUFFER_SPRITE), &m_pConstantBufferSprite2D);

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

