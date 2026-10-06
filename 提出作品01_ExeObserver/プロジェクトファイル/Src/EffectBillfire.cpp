#include "EffectManager.h"
#include "AudioManager.h"

namespace {
	const int EFFECT_ANIM_SPEED = 5;     // アニメーションの速さ
	const int EFFECT_MAX = 500;

}

EffectBillfire::EffectBillfire()
{
	ObjectManager::SetDrawOrder(this, -200);   // 透明色のため最後に描画する

	uvOffset = VECTOR2(0,0);
	frame = 0;

	EffectManager* efmObj = ObjectManager::FindGameObject<EffectManager>();
	mesh = efmObj->Mesh();
	billB = efmObj->BillboardList("");

	AudioManager::Audio("SeNitro")->Play();

}
EffectBillfire::~EffectBillfire()
{
}

void EffectBillfire::SetEffectName(std::string name)
{
	billB = ObjectManager::FindGameObject<EffectManager>()->BillboardList(name);
}

void EffectBillfire::Update()
{
	int idx;

	frame += 60 * SceneManager::DeltaTime();
	idx = (int)(frame / EFFECT_ANIM_SPEED);

	if (idx >= billB->m_dwNumX * billB->m_dwNumY)	// パターンの最後に達したとき
	{
		if (Parent() == nullptr)
		{
			DestroyMe();
			return;
		}
		else {
			frame = 0;	  // 親があるときは、削除は親が行う
		}
	}

	// アニメーションのためのテクスチャオフセットを設定する
	uvOffset.x = (float)(idx % billB->m_dwNumX * billB->m_dwSrcWidth);   // Ｘ方向の変位
	uvOffset.y = (float)(idx / billB->m_dwNumX * billB->m_dwSrcHeight);  // Ｙ方向の変位

}

void EffectBillfire::Draw()
{

	//使用するシェーダーのセット
	GameDevice()->m_pD3D->m_pDeviceContext->VSSetShader(GameDevice()->m_pShader->m_pEffect3D_VS_BILL.Get(), NULL, 0);
	GameDevice()->m_pD3D->m_pDeviceContext->PSSetShader(GameDevice()->m_pShader->m_pEffect3D_PS.Get(), NULL, 0);

	//ビルボードの、視点を向くワールドトランスフォームを求める
	MATRIX4X4 mWorld = GetLookatMatrix(transform.position, GameDevice()->m_vEyePt);
	// 描画中心位置の移動をする
	MATRIX4X4 mPosUp = XMMatrixTranslation(billB->m_fDestCenterX - billB->m_fDestWidth / 2,
		billB->m_fDestCenterY - billB->m_fDestHeight / 2, 0);
	mWorld = mPosUp * mWorld;

	//バーテックスバッファーをセット
	UINT stride = sizeof(BILLBOARD_VERTEX);
	UINT offset = 0;
	ID3D11Buffer* pVertexBuffer = billB->m_pVertexBuffer.Get();   // バーテックスバッファ
	GameDevice()->m_pD3D->m_pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

	UINT mask = 0xffffffff;
	if (billB->m_nBlendFlag == 1)   // 加算合成色指定
	{
		// 加算合成色のブレンディングを設定
		GameDevice()->m_pD3D->m_pDeviceContext->OMSetBlendState(GameDevice()->m_pD3D->m_pBlendStateAdd.Get(), NULL, mask);
	}

	//シェーダーのコンスタントバッファーに各種データを渡す
	D3D11_MAPPED_SUBRESOURCE pData;
	CONSTANT_BUFFER_EFFECT cb;
	ZeroMemory(&cb, sizeof(cb));

	if (SUCCEEDED(GameDevice()->m_pD3D->m_pDeviceContext->Map(GameDevice()->m_pShader->m_pConstantBufferEffect.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		//ワールド、カメラ、射影行列、テクスチャーオフセットを渡す
		cb.mWVP = XMMatrixTranspose(mWorld * GameDevice()->m_mView * GameDevice()->m_mProj);

		cb.vUVOffset.x = uvOffset.x / billB->m_dwImageWidth;		// テクスチャアニメーションのオフセット
		cb.vUVOffset.y = uvOffset.y / billB->m_dwImageHeight;		// テクスチャアニメーションのオフセット
		cb.vUVScale.x = 1;
		cb.vUVScale.y = 1;
		cb.fAlpha = billB->m_fAlpha;

		memcpy_s(pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb));
		GameDevice()->m_pD3D->m_pDeviceContext->Unmap(GameDevice()->m_pShader->m_pConstantBufferEffect.Get(), 0);
	}
	//このコンスタントバッファーをどのシェーダーで使うか
	GameDevice()->m_pD3D->m_pDeviceContext->VSSetConstantBuffers(0, 1, GameDevice()->m_pShader->m_pConstantBufferEffect.GetAddressOf());
	GameDevice()->m_pD3D->m_pDeviceContext->PSSetConstantBuffers(0, 1, GameDevice()->m_pShader->m_pConstantBufferEffect.GetAddressOf());
	//頂点インプットレイアウトをセット
	GameDevice()->m_pD3D->m_pDeviceContext->IASetInputLayout(GameDevice()->m_pShader->m_pEffect3D_VertexLayout_BILL.Get());
	//プリミティブ・トポロジーをセット
	GameDevice()->m_pD3D->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//テクスチャーをシェーダーに渡す
	GameDevice()->m_pD3D->m_pDeviceContext->PSSetSamplers(0, 1, GameDevice()->m_pD3D->m_pSampleLinear.GetAddressOf());
	GameDevice()->m_pD3D->m_pDeviceContext->PSSetShaderResources(0, 1, billB->m_pTexture.GetAddressOf());
	//プリミティブをレンダリング
	GameDevice()->m_pD3D->m_pDeviceContext->Draw(4, 0);

	if (billB->m_nBlendFlag == 1)   // 加算合成色指定
	{
		// 通常のブレンディングに戻す
		GameDevice()->m_pD3D->m_pDeviceContext->OMSetBlendState(GameDevice()->m_pD3D->m_pBlendStateTrapen.Get(), NULL, mask);
	}

}
