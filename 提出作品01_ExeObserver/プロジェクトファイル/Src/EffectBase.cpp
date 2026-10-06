#include "EffectManager.h"
#include "EffectBase.h"
#include "Player.h"

EffectBase::EffectBase()
{
}

EffectBase::~EffectBase()
{
}

bool EffectBase::LoadBillTexture(const TCHAR* szFName, BILLBOARDBASE* pBillBase)
{
	//テクスチャー読み込み	
	if (FAILED(GameDevice()->m_pD3D->CreateShaderResourceViewFromFile(szFName, pBillBase->m_pTexture.ReleaseAndGetAddressOf(),
						pBillBase->m_dwImageWidth, pBillBase->m_dwImageHeight, 3)))
	{
		MessageBox(0, _T("ビルボード　テクスチャーを読み込めません"), szFName, MB_OK);
		return false;
	}

	return true;
}

bool EffectBase::SetBillSrc(BILLBOARDBASE* pBillBase)
{
	// ビルボード用の板ポリゴンを作成し、バーテックスバッファを作成する

	//バーテックスバッファー作成(裏向きの座標で作成)
	BILLBOARD_VERTEX vertices[] =
	{
		VECTOR3(-pBillBase->m_fDestWidth / 2,  pBillBase->m_fDestHeight / 2, 0), VECTOR2((float)(pBillBase->m_dwSrcX + pBillBase->m_dwSrcWidth) / pBillBase->m_dwImageWidth, (float)pBillBase->m_dwSrcY / pBillBase->m_dwImageHeight),                     //頂点1  左上
		VECTOR3(-pBillBase->m_fDestWidth / 2, -pBillBase->m_fDestHeight / 2, 0), VECTOR2((float)(pBillBase->m_dwSrcX + pBillBase->m_dwSrcWidth) / pBillBase->m_dwImageWidth, (float)(pBillBase->m_dwSrcY + pBillBase->m_dwSrcHeight) / pBillBase->m_dwImageHeight),   //頂点2　左下	  // -- 2024.3.23
		VECTOR3(pBillBase->m_fDestWidth / 2,  pBillBase->m_fDestHeight / 2, 0), VECTOR2((float)pBillBase->m_dwSrcX / pBillBase->m_dwImageWidth,                (float)pBillBase->m_dwSrcY / pBillBase->m_dwImageHeight),                        //頂点2　右上					      // -- 2024.3.23
		VECTOR3(pBillBase->m_fDestWidth / 2, -pBillBase->m_fDestHeight / 2, 0), VECTOR2((float)pBillBase->m_dwSrcX / pBillBase->m_dwImageWidth,                (float)(pBillBase->m_dwSrcY + pBillBase->m_dwSrcHeight) / pBillBase->m_dwImageHeight),      //頂点4　右下
	};

	// ２度目以降に初期化されることの対策
	pBillBase->m_pVertexBuffer.Reset();

	// バーテックスバッファー作成
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(BILLBOARD_VERTEX) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;
	if (FAILED(GameDevice()->m_pD3D->m_pDevice->CreateBuffer(&bd, &InitData, pBillBase->m_pVertexBuffer.ReleaseAndGetAddressOf())))
	{
		return false;
	}

	return true;
}


bool EffectBase::LoadPartTexture(const TCHAR* szFName, PARTICLEBASE* pPartBase)
{
	//テクスチャー読み込み	
	if (FAILED(GameDevice()->m_pD3D->CreateShaderResourceViewFromFile(szFName, pPartBase->m_pTexture.ReleaseAndGetAddressOf(),
		pPartBase->m_dwImageWidth, pPartBase->m_dwImageHeight, 3)))
	{
		MessageBox(0, _T("パーティクル　テクスチャーを読み込めません"), szFName, MB_OK);
		return false;
	}

	return true;
}

bool EffectBase::SetPartSrc(PARTICLEBASE* pPartBase)
{
	//バーテックス
	PARTICLE_VERTEX vertices[] =
	{
		VECTOR3(0.0f, 0.0f, 0.0f)
	};

	// ２度目以降に初期化されることの対策
	pPartBase->m_pVertexBuffer.Reset();

	// バーテックスバッファー作成
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PARTICLE_VERTEX) * 1;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;
	if (FAILED(GameDevice()->m_pD3D->m_pDevice->CreateBuffer(&bd, &InitData, pPartBase->m_pVertexBuffer.ReleaseAndGetAddressOf())))
	{
		return false;
	}

	return true;

}
