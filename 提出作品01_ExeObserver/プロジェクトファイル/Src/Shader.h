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
//		・トゥーンシェーディング用のシェーダー          InitShaderToon();
//		・輪郭線用のシェーダー                          InitShaderOutline();
//
//                                                                              Shader.h
// ========================================================================================
#pragma once

//ヘッダーファイルのインクルード
#include <stdio.h>
#include <windows.h>
#include <wrl/client.h>

#include "Main.h"
#include "Direct3D.h"

//シェーダーのバッファ構造体定義

// メッシュシェーダー用のコンスタントバッファーのアプリ側構造体。
// （ワールド行列から射影行列、ライト、カラー）  // -- 2020.1.24
struct CONSTANT_BUFFER_WVLED
{
	MATRIX4X4  mW;             // ワールド行列
	MATRIX4X4  mWVP;           // ワールドから射影までの変換行列
	VECTOR4    vLightDir;      // ライト方向
	VECTOR4    vEyePos;        // 視点
	VECTOR4    vDiffuse;       // ディフューズ色	
	VECTOR4    vDrawInfo;      // 描画関連情報(使用していない)   // -- 2020.12.15
	CONSTANT_BUFFER_WVLED()
	{
		ZeroMemory(this, sizeof(CONSTANT_BUFFER_WVLED));
	}
};

// ディスプレースメントマッピング用の各種データを渡す  // -- 2020.1.24
struct CONSTANT_BUFFER_DISPLACE
{
	VECTOR3    vEyePosInv;    // 各頂点から見た、視点の位置
	float      fMinDistance;  // ポリゴン分割の最小距離
	float      fMaxDistance;  // ポリゴン分割の最大小距離
	int        iMaxDevide;    // 分割最大数
	VECTOR2    vHeight;       // ディスプレースメントマッピング時の盛り上げ高さ
	VECTOR4    vWaveMove;     // 波の移動量(波の処理時のみ)
	VECTOR4    vSpecular;     // 鏡面反射(波の処理時のみ)
	CONSTANT_BUFFER_DISPLACE()
	{
		ZeroMemory(this, sizeof(CONSTANT_BUFFER_DISPLACE));
	}
};

//  エフェクト用のコンスタントバッファのアプリ側構造体   //  2017.8.25
struct CONSTANT_BUFFER_EFFECT
{
	MATRIX4X4  mWVP;       // ワールドから射影までの変換行列
	MATRIX4X4  mW;         // ワールド
	MATRIX4X4  mV;         // ビュー
	MATRIX4X4  mP;         // 射影
	VECTOR2    vUVOffset;  // テクスチャ座標のオフセット
	VECTOR2    vUVScale;   // テクスチャ座標の拡縮  // -- 2019.7.17
	float      fAlpha;
	float      fSize;      // パーティクルの大きさ  // -- 2018.8.23
	VECTOR2    Dummy;                               // -- 2019.7.17
	CONSTANT_BUFFER_EFFECT()
	{
		ZeroMemory(this, sizeof(CONSTANT_BUFFER_EFFECT));
	}
};


//　3Dスプライトシェーダー用のコンスタントバッファーのアプリ側構造体 
struct CONSTANT_BUFFER_SPRITE
{
	MATRIX4X4  mWVP;
	MATRIX4X4  mW;
	float      ViewPortWidth;
	float      ViewPortHeight;
	VECTOR2    vUVOffset;
	VECTOR4    vColor;         // カラー情報。半透明の割合を指定する
	VECTOR4    vMatInfo;       // マテリアル関連情報　x:テクスチャ有り無し。DrawRect()、DrawLine()で使用。
	CONSTANT_BUFFER_SPRITE()
	{
		ZeroMemory(this, sizeof(CONSTANT_BUFFER_SPRITE));
	}
};

// マテリアル情報                      // -- 2020.12.15
struct CONSTANT_BUFFER_MATERIAL
{
	VECTOR4    vMatDuffuse;
	VECTOR4    vMatSpecular;
};

// 輪郭線用のコンスタントバッファーのアプリ側構造体
// （HLSL 側 cbOutline : register(b3) と同じレイアウトにすること）
struct CONSTANT_BUFFER_OUTLINE
{
	VECTOR4    vOutlineColor;   // 輪郭線の色
	float      fOutlineWidth;   // 輪郭線の太さ(NDC単位。縦方向基準)
	float      fOutlineAspect;  // 画面のアスペクト比(幅 / 高さ)
	float      fPad[2];         // パディング
	CONSTANT_BUFFER_OUTLINE()
	{
		ZeroMemory(this, sizeof(CONSTANT_BUFFER_OUTLINE));
	}
};

// トゥーンシェーディング用のコンスタントバッファーのアプリ側構造体
// （HLSL 側 cbToon : register(b4) と同じレイアウトにすること）
struct CONSTANT_BUFFER_TOON
{
	VECTOR4    vShadowColor;    // 影部分に掛ける色
	float      fThreshold;      // 明暗の境界となる内積値
	float      fSmoothFactor;   // 境界のぼかし幅
	float      fRimPower;       // リムライトの絞り(指数)
	float      fRimStrength;    // リムライトの寄与率
	CONSTANT_BUFFER_TOON()
	{
		ZeroMemory(this, sizeof(CONSTANT_BUFFER_TOON));
	}
};

//
// CShaderクラス
//
class CShader
{
public:
	// Direct3D11
	CDirect3D*              m_pD3D;

	// シェーダー
	// 通常用のシンプルなシェーダー
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_pSimple_VertexLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pSimple_VS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pSimple_PS;

	// 3Dスプライト用のシェーダー
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_pSprite3D_VertexLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pSprite3D_VS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pSprite3D_PS;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pSprite3D_VS_BILL;

	// ディスプレースメントマッピング(波)用のシェーダー
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_pDisplaceWave_VertexLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pDisplaceWave_VS;
	Microsoft::WRL::ComPtr<ID3D11HullShader>       m_pDisplaceWave_HS;
	Microsoft::WRL::ComPtr<ID3D11DomainShader>     m_pDisplaceWave_DS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pDisplaceWave_PS;

	// ディスプレースメントマッピング(スキンメッシュ)用のシェーダー
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pDisplaceSkinMesh_VS;
	Microsoft::WRL::ComPtr<ID3D11HullShader>       m_pDisplaceSkinMesh_HS;
	Microsoft::WRL::ComPtr<ID3D11DomainShader>     m_pDisplaceSkinMesh_DS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pDisplaceSkinMesh_PS;

	// ディスプレースメントマッピング(スタティックメッシュ)用のシェーダー
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pDisplaceStaticMesh_VS;
	Microsoft::WRL::ComPtr<ID3D11HullShader>       m_pDisplaceStaticMesh_HS;
	Microsoft::WRL::ComPtr<ID3D11DomainShader>     m_pDisplaceStaticMesh_DS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pDisplaceStaticMesh_PS;

	// エフェクト用のシェーダー
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_pEffect3D_VertexLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pEffect3D_VS_POINT;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader>   m_pEffect3D_GS_POINT;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pEffect3D_PS;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_pEffect3D_VertexLayout_BILL;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pEffect3D_VS_BILL;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pEffect3D_VS_BILLMESH;  // -- 2019.7.17

	// Fbxモデル　スタティツクメッシュ用のシェーダー
	// （Normalマッピング）
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_pFbxStaticMesh_VertexLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pFbxStaticMesh_VS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pFbxStaticMesh_PS;

	// Fbxモデル　スキンメッシュ用のシェーダー
	// （Normalマッピング）
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_pFbxSkinMesh_VertexLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pFbxSkinMesh_VS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pFbxSkinMesh_PS;

	// トゥーンシェーディング用のシェーダー
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_pToon_VertexLayout;      // スタティックメッシュ用
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pToon_VS;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_pToonSkin_VertexLayout;  // スキンメッシュ用
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pToonSkin_VS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pToon_PS;                // スタティック／スキン共通

	// 輪郭線用のシェーダー
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_pOutline_VertexLayout;      // スタティックメッシュ用
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pOutline_VS;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_pOutlineSkin_VertexLayout;  // スキンメッシュ用
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_pOutlineSkin_VS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pOutline_PS;                // スタティック／スキン共通


	// コンスタントバッファ  ------------------------------------------

	// コンスタントバッファーディスプレイスメントマッピング用   // -- 2020.1.24
	Microsoft::WRL::ComPtr<ID3D11Buffer>           m_pConstantBufferDisplace;

	// コンスタントバッファーエフェクト用
	Microsoft::WRL::ComPtr<ID3D11Buffer>           m_pConstantBufferEffect;

	// コンスタントバッファー 3Dスプライト用
	Microsoft::WRL::ComPtr<ID3D11Buffer>			m_pConstantBufferSprite3D;

	// コンスタントバッファー　メッシュ 変換行列・カラー渡し用
	Microsoft::WRL::ComPtr<ID3D11Buffer>           m_pConstantBufferWVLED;    // -- 2020.1.24

	// コンスタントバッファー　ボーン行列渡し用
	Microsoft::WRL::ComPtr<ID3D11Buffer>           m_pConstantBufferBone2;

	// マテリアル情報　渡し用
	Microsoft::WRL::ComPtr<ID3D11Buffer>           m_pConstantBufferMaterial;    // -- 2020.12.15

	// コンスタントバッファー　輪郭線パラメータ渡し用(VS b3)
	Microsoft::WRL::ComPtr<ID3D11Buffer>           m_pConstantBufferOutline;

	// コンスタントバッファー　トゥーンパラメータ渡し用(PS b4)
	Microsoft::WRL::ComPtr<ID3D11Buffer>           m_pConstantBufferToon;


public:
	HRESULT InitShader();
	HRESULT InitShaderSimple();
	HRESULT InitShaderSprite();

	HRESULT InitShaderFbx();
	HRESULT InitShaderDisplace();
	HRESULT InitShaderEffect();
	HRESULT InitShaderToon();
	HRESULT InitShaderOutline();
	HRESULT InitShaderConstant();

	HRESULT MakeShader(const TCHAR ProfileName[], const TCHAR FileName[], void** ppShader, D3D11_INPUT_ELEMENT_DESC Fluid_layout[] = nullptr, UINT numElements = 0, ID3D11InputLayout** ppInputLayout = nullptr);
	HRESULT MakeConstantBuffer(UINT size, ID3D11Buffer**  ppConstantBuffer);

	CShader(CDirect3D* pD3D);
	~CShader();

};
