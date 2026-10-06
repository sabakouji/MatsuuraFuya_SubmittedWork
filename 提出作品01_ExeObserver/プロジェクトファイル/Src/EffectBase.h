#pragma once
#include "Object3D.h"
#include "Animator.h"
#include <wrl/client.h>

class EffectBase : public Object3D
{
public:
	// ========================================================================================
	//
	// 頂点の構造体
	//
	// ========================================================================================
	struct PARTICLE_VERTEX
	{
		VECTOR3 Pos; //位置
	};
	struct BILLBOARD_VERTEX
	{
		VECTOR3 Pos; //位置
		VECTOR2 UV;  //テクスチャー座標
	};

	// ========================================================================================
	//
	// 一つのオブジェクトの中のパーティクル構造体
	//
	// ========================================================================================
	struct PART
	{
		VECTOR3     Pos;
		VECTOR3     Dir;
		float       Speed;
		int         BirthFrame;
		PART()
		{
			ZeroMemory(this, sizeof(PART));
		}
	};

	// ========================================================================================
	//
	//	パーティクル配列用構造体
	// 
	// ========================================================================================
	struct PARTICLEBASE
	{
		std::string                 m_name;			   // 別名
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_pTexture;        // テクスチャー(１つのテクスチャーには1つのパーティクルのみ)
		DWORD						m_dwImageWidth;    // テクスチャーの幅
		DWORD						m_dwImageHeight;   // テクスチャーの高さ
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_pVertexBuffer;   // バーテックスバッファ
		int							m_nNum;            // 一つのオブジェクト中のパーティクル数
		float						m_fDestSize;       // 表示サイズ(幅と高さは同一)
		int							m_FrameEnd;        // パーティクルを表示している時間
		float						m_fSpeed;          // パーティクルの移動スピード
		int							m_iBarthFrame;     // パーティクルの開始までの最大待ち時間。０は待ち無し
		int							m_ifBound;         // 地面でバウンドさせるか（0:バウンドなし 1:地面でバウンド）
		float						m_fAlpha;          // 透明度
		int							m_nBlendFlag;      // 0:通常 1:自己発光
	};

	// ========================================================================================
	//
	//	ビルボード構造体
	// 
	// ========================================================================================
	struct BILLBOARDBASE
	{
		std::string                 m_name;			   // 別名
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_pTexture;        // テクスチャー
		DWORD						m_dwImageWidth;    // テクスチャーの幅
		DWORD						m_dwImageHeight;   // テクスチャーの高さ
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_pVertexBuffer;   // バーテックスバッファ
		float						m_fDestWidth;      // 表示幅
		float						m_fDestHeight;     // 表示高さ
		float						m_fDestCenterX;    // 表示中心位置Ｘ
		float						m_fDestCenterY;    // 表示中心位置Ｙ
		DWORD						m_dwSrcX;          // パターンの位置Ｘ
		DWORD						m_dwSrcY;          // パターンの位置Ｙ
		DWORD						m_dwSrcWidth;      // １つのパターンの幅
		DWORD						m_dwSrcHeight;     // １つのパターンの高さ
		DWORD						m_dwNumX;          // パターンの横の数
		DWORD						m_dwNumY;          // パターンの縦の数
		float						m_fAlpha;          // 透明度
		int							m_nBlendFlag;      // 0:通常  1:自己発光
		int							m_nDrawFlag;       // 0:ビルボード  1:ビルボードメッシュ
	};

public:
	EffectBase();
	virtual ~EffectBase();

	void SetNormal(VECTOR3 normalIn) { normal = normalIn; }

	bool LoadBillTexture(const TCHAR* szFName, BILLBOARDBASE* pBillBase);
	bool SetBillSrc(BILLBOARDBASE* pBillBase);
	bool LoadPartTexture(const TCHAR* szFName, PARTICLEBASE* pPartBase);
	bool SetPartSrc(PARTICLEBASE* pPartBase);

protected:
	VECTOR3 normal;
};