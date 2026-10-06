#include "EffectManager.h"
#include "AudioManager.h"

namespace {
	const int EFFECT_ANIM_SPEED = 5;     // アニメーションの速さ

}

EffectBillboard::EffectBillboard()
{
	ObjectManager::SetDrawOrder(this, -200);   // 透明色のため最後に描画する

	uvOffset = VECTOR2(0,0);
	frame = 0;

	EffectManager* efmObj = ObjectManager::FindGameObject<EffectManager>();
	mesh = efmObj->Mesh();
	billB = efmObj->BillboardList("");

	AudioManager::Audio("SeNitro")->Play();

}
EffectBillboard::~EffectBillboard()
{
}

void EffectBillboard::SetEffectName(std::string name)
{
	billB = ObjectManager::FindGameObject<EffectManager>()->BillboardList(name);
}

void EffectBillboard::Update()
{
	int idx;

	frame += 60 * SceneManager::DeltaTime();
	idx = (int)(frame / EFFECT_ANIM_SPEED);

	if (idx >= billB->m_dwNumX * billB->m_dwNumY)	// パターンの最後に達したとき
	{
		DestroyMe();
		return;
	}

	// アニメーションのためのテクスチャオフセットを設定する
	uvOffset.x = (float)(idx % billB->m_dwNumX * billB->m_dwSrcWidth);   // Ｘ方向の変位
	uvOffset.y = (float)(idx / billB->m_dwNumX * billB->m_dwSrcHeight);  // Ｙ方向の変位

}

void EffectBillboard::Draw()
{
	//使用するシェーダーの登録	
	GameDevice()->m_pD3D->m_pDeviceContext->VSSetShader(GameDevice()->m_pShader->m_pEffect3D_VS_BILLMESH.Get(), NULL, 0);
	GameDevice()->m_pD3D->m_pDeviceContext->PSSetShader(GameDevice()->m_pShader->m_pEffect3D_PS.Get(), NULL, 0);

	//ビルボードの、視点を向くワールドトランスフォームを求める
	MATRIX4X4 mWorld = GetLookatMatrix(transform.position, GameDevice()->m_vEyePt);
	// 描画中心位置の移動をする
	MATRIX4X4 mPosUp = XMMatrixTranslation(billB->m_fDestCenterX - billB->m_fDestWidth / 2,
							billB->m_fDestCenterY - billB->m_fDestHeight / 2, 0);
	mWorld = mPosUp * mWorld;
	// メッシュの大きさに拡大する設定をする（元々のメッシュは１ｍ四方の半球）
	MATRIX4X4 mScale = XMMatrixScaling(billB->m_fDestWidth, billB->m_fDestHeight, 
							(billB->m_fDestWidth + billB->m_fDestHeight) / 2);
	mWorld = mScale * mWorld;

	UINT mask = 0xffffffff;
	if (billB->m_nBlendFlag == 1)   // 加算合成色指定
	{
		// 加算合成色のブレンディングを設定
		GameDevice()->m_pD3D->m_pDeviceContext->OMSetBlendState(GameDevice()->m_pD3D->m_pBlendStateAdd.Get(), NULL, mask);
	}

	//シェーダーのコンスタントバッファーに各種データを渡す	
	D3D11_MAPPED_SUBRESOURCE pData;
	CONSTANT_BUFFER_EFFECT cb;
	if (SUCCEEDED(GameDevice()->m_pD3D->m_pDeviceContext->Map(GameDevice()->m_pShader->m_pConstantBufferEffect.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		//ワールド、カメラ、射影行列、テクスチャーオフセットを渡す
		cb.mWVP = XMMatrixTranspose(mWorld * GameDevice()->m_mView * GameDevice()->m_mProj);

		cb.vUVOffset.x = uvOffset.x /  billB->m_dwImageWidth;			// テクスチャアニメーションのオフセット
		cb.vUVOffset.y = uvOffset.y /  billB->m_dwImageHeight;			// テクスチャアニメーションのオフセット
		cb.vUVScale.x = (float) billB->m_dwSrcWidth /  billB->m_dwImageWidth;		// テクスチャアニメーションの1/横回数
		cb.vUVScale.y = (float) billB->m_dwSrcHeight /  billB->m_dwImageHeight;	// テクスチャアニメーションの1/縦回数

		cb.fAlpha =  billB->m_fAlpha;

		memcpy_s(pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb));
		cb.mWVP = XMMatrixTranspose(mWorld * GameDevice()->m_mView * GameDevice()->m_mProj);
		cb.mWVP = XMMatrixTranspose(mWorld * GameDevice()->m_mView * GameDevice()->m_mProj);
		cb.mWVP = XMMatrixTranspose(mWorld * GameDevice()->m_mView * GameDevice()->m_mProj);
		cb.mWVP = XMMatrixTranspose(mWorld * GameDevice()->m_mView * GameDevice()->m_mProj);
		GameDevice()->m_pD3D->m_pDeviceContext->Unmap(GameDevice()->m_pShader->m_pConstantBufferEffect.Get(), 0);
	}

	//このコンスタントバッファーをどのシェーダーで使うか
	GameDevice()->m_pD3D->m_pDeviceContext->VSSetConstantBuffers(0, 1, GameDevice()->m_pShader->m_pConstantBufferEffect.GetAddressOf());
	GameDevice()->m_pD3D->m_pDeviceContext->PSSetConstantBuffers(0, 1, GameDevice()->m_pShader->m_pConstantBufferEffect.GetAddressOf());

	//頂点インプットレイアウトをセット
	GameDevice()->m_pD3D->m_pDeviceContext->IASetInputLayout(GameDevice()->m_pShader->m_pFbxStaticMesh_VertexLayout.Get());

	//プリミティブ・トポロジーをセット
	GameDevice()->m_pD3D->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//テクスチャーサンプラーをシェーダーに渡す
	GameDevice()->m_pD3D->m_pDeviceContext->PSSetSamplers(0, 1, GameDevice()->m_pD3D->m_pSampleLinear.GetAddressOf());

	// メッシュの描画順を決定するm_dwRenderIdxArrayの設定
	mesh->SetRenderIdxArray(mWorld, GameDevice()->m_vEyePt);

	// メッシュの数だけテクスチャー、バーテックスバッファ、インデックスバッファをセットして、レンダリングする
	for (DWORD mi = 0; mi < mesh->m_dwMeshNum; mi++)
	{
		DWORD i = mesh->m_RenderOrder[mi].Idx;		// -- 2025.3.15

		//テクスチャーをシェーダーに渡す
		GameDevice()->m_pD3D->m_pDeviceContext->PSSetShaderResources(0, 1, billB->m_pTexture.GetAddressOf());

		// バーテックスバッファーをセット
		UINT stride = sizeof(StaticVertexNormal);
		UINT offset = 0;
		GameDevice()->m_pD3D->m_pDeviceContext->IASetVertexBuffers(0, 1, mesh->m_pMeshArray[i].m_pVertexBuffer.GetAddressOf(), &stride, &offset);

		//インデックスバッファーをセット
		GameDevice()->m_pD3D->m_pDeviceContext->IASetIndexBuffer(mesh->m_pMeshArray[i].m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//プリミティブをレンダリング
		// （インデックスの数を指定してレンダリング）
		GameDevice()->m_pD3D->m_pDeviceContext->DrawIndexed(mesh->m_pMeshArray[i].m_dwIndicesNum, 0, 0);
	}

	if ( billB->m_nBlendFlag == 1)   // 加算合成色指定
	{
		// 通常のブレンディングに戻す
		GameDevice()->m_pD3D->m_pDeviceContext->OMSetBlendState(GameDevice()->m_pD3D->m_pBlendStateTrapen.Get(), NULL, mask);
	}

}
