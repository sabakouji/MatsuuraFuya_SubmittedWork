#include "EffectManager.h"

EffectManager::EffectManager()
{
	ObjectManager::DontDestroy(this);		// 自体は消されない
	ObjectManager::SetVisible(this, true);		// ECSパーティクル描画のためDrawを有効化
	ObjectManager::SetDrawOrder(this, -200);	// 透明加算のため最後に描画

	// 半球ビルボードスタティックメッシュの読み込み
	mesh = new CFbxMesh();
	mesh->Load("Data/Item/BillSphere.mesh");
	meshCol = nullptr;

	// ------------------------------------------------------------------------------------------
	// ビルボード構造体リストの設定
	BILLBOARDBASE bb = {};

	// 炎ビルボード "sparklen3" (先頭要素:省略値)
	billboardList.push_back(bb);
	billboardList.back().m_name = "sparklen3";
	LoadBillTexture("Data/Image/sparklen3.png", &billboardList.back());
	billboardList.back().m_fDestWidth = 0.7f;      // 表示幅
	billboardList.back().m_fDestHeight = 0.7f;     // 表示高さ
	billboardList.back().m_fDestCenterX = billboardList.back().m_fDestWidth / 2;  // 表示中心位置Ｘ(真ん中)
	billboardList.back().m_fDestCenterY = billboardList.back().m_fDestHeight / 2; // 表示中心位置Ｙ(真ん中)
	billboardList.back().m_dwSrcX = 0;             // パターンの位置　Ｘ座標
	billboardList.back().m_dwSrcY = 0;             // パターンの位置　Ｙ座標
	billboardList.back().m_dwSrcWidth = 85;        // パターンの幅
	billboardList.back().m_dwSrcHeight = 85;       // パターンの高さ
	billboardList.back().m_dwNumX = 3;             // アニメーションさせるパターンの数　Ｘ方向
	billboardList.back().m_dwNumY = 1;             // アニメーションさせるパターンの数　Ｙ方向
	billboardList.back().m_fAlpha = 0.9f;          // 透明度
	billboardList.back().m_nBlendFlag = 1;         // ブレンドステートフラグ(0:通常描画　1:加算合成色描画)
	billboardList.back().m_nDrawFlag = 0;          // 描画フラグ(0:ビルボードのみ)
	SetBillSrc(&billboardList.back());                 // バーテックスバッファの作成

	// 爆発ビルボード	"Bom3"
	billboardList.push_back(bb);
	billboardList.back().m_name = "Bom3";
	LoadBillTexture("Data/Image/Bom3.png", &billboardList.back());
	billboardList.back().m_fDestWidth = 4.0f;      // 表示幅
	billboardList.back().m_fDestHeight = 4.0f;     // 表示高さ
	billboardList.back().m_fDestCenterX = billboardList.back().m_fDestWidth / 2;  // 表示中心位置Ｘ(真ん中)
	billboardList.back().m_fDestCenterY = billboardList.back().m_fDestHeight / 2; // 表示中心位置Ｙ(真ん中)
	billboardList.back().m_dwSrcX = 0;             // パターンの位置　Ｘ座標
	billboardList.back().m_dwSrcY = 0;             // パターンの位置　Ｙ座標
	billboardList.back().m_dwSrcWidth = 64;        // パターンの幅
	billboardList.back().m_dwSrcHeight = 64;       // パターンの高さ
	billboardList.back().m_dwNumX = 4;             // アニメーションさせるパターンの数　Ｘ方向
	billboardList.back().m_dwNumY = 4;             // アニメーションさせるパターンの数　Ｙ方向
	billboardList.back().m_fAlpha = 0.9f;          // 透明度
	billboardList.back().m_nBlendFlag = 1;         // ブレンドステートフラグ(0:通常描画　1:加算合成色描画)
	billboardList.back().m_nDrawFlag = 1;          // 描画フラグ(0:ビルボード　1:ビルボードメッシュ)
	SetBillSrc(&billboardList.back());                 // バーテックスバッファの作成

	// 爆発ビルボード2	  "Bom4"
	billboardList.push_back(bb);
	billboardList.back().m_name = "Bom4";
	LoadBillTexture("Data/Image/Bom4.png", &billboardList.back());
	billboardList.back().m_fDestWidth = 4.0f;      // 表示幅
	billboardList.back().m_fDestHeight = 4.0f;     // 表示高さ
	billboardList.back().m_fDestCenterX = billboardList.back().m_fDestWidth / 2;  // 表示中心位置Ｘ(真ん中)
	billboardList.back().m_fDestCenterY = billboardList.back().m_fDestHeight;   // 表示中心位置Ｙ(下の端)
	billboardList.back().m_dwSrcX = 0;             // パターンの位置　Ｘ座標
	billboardList.back().m_dwSrcY = 0;             // パターンの位置　Ｙ座標
	billboardList.back().m_dwSrcWidth = 128;       // パターンの幅
	billboardList.back().m_dwSrcHeight = 128;      // パターンの高さ
	billboardList.back().m_dwNumX = 4;             // アニメーションさせるパターンの数　Ｘ方向
	billboardList.back().m_dwNumY = 4;             // アニメーションさせるパターンの数　Ｙ方向
	billboardList.back().m_fAlpha = 0.9f;          // 透明度
	billboardList.back().m_nBlendFlag = 0;         // ブレンドステートフラグ(0:通常描画　1:加算合成色描画)
	billboardList.back().m_nDrawFlag = 1;          // 描画フラグ(0:ビルボード　1:ビルボードメッシュ)
	SetBillSrc(&billboardList.back());                 // バーテックスバッファの作成

	// ------------------------------------------------------------------------------------------
	// パーティクル構造体の設定
	PARTICLEBASE pb = {};

	// 火花パーティクル "particle3"(先頭要素:省略値)
	particleList.push_back(pb);
	particleList.back().m_name = "particle3";
	LoadPartTexture(_T("Data/Image/particle3.png"), &particleList.back());// パーティクルテクスチャ
	particleList.back().m_nNum = 100;             // 一つのオブジェクト中のパーティクル数。PARTICLE_NUM_MAX以下であること。
	particleList.back().m_fDestSize = 0.1f;       // 表示サイズ(一つのパーティクルの大きさ)
	particleList.back().m_FrameEnd = 60;          // パーティクルを表示している時間
	particleList.back().m_fSpeed = 0.015f;        // パーティクルの移動スピード。ランダム
	particleList.back().m_iBarthFrame = 20;       // パーティクルの開始までの最大待ち時間。ランダムで開始。０は待ち無し
	particleList.back().m_ifBound = 0;            // 地面でバウンドさせるか（0:バウンドなし 1:地面でバウンド）
	particleList.back().m_fAlpha = 0.9f;          // 透明度
	particleList.back().m_nBlendFlag = 1;         // ブレンドステートフラグ(0:通常描画　1:加算合成色描画)
	SetPartSrc(&particleList.back());             // バーテックスバッファの作成

	// 火花パーティクル・大  "particle2"
	particleList.push_back(pb);
	particleList.back().m_name = "particle2";
	LoadPartTexture(_T("Data/Image/particle2.png"), &particleList.back());// パーティクルテクスチャ
	particleList.back().m_nNum = 10;              // 一つのオブジェクト中のパーティクル数。PARTICLE_NUM_MAX以下であること。
	particleList.back().m_fDestSize = 0.5f;       // 表示サイズ(一つのパーティクルの大きさ)
	particleList.back().m_FrameEnd = 60;          // パーティクルを表示している時間
	particleList.back().m_fSpeed = 0.015f;        // パーティクルの移動スピード。ランダム
	particleList.back().m_iBarthFrame = 0;        // パーティクルの開始までの最大待ち時間。ランダムで開始。０は待ち無し
	particleList.back().m_ifBound = 0;            // 地面でバウンドさせるか（0:バウンドなし 1:地面でバウンド）
	particleList.back().m_fAlpha = 0.9f;          // 透明度
	particleList.back().m_nBlendFlag = 1;         // ブレンドステートフラグ(0:通常描画　1:加算合成色描画)
	SetPartSrc(&particleList.back());             // バーテックスバッファの作成

}

EffectManager::~EffectManager()
{
	SAFE_DELETE(mesh);
	for (BILLBOARDBASE& bb : billboardList )
	{
		bb.m_pTexture.Reset();
		bb.m_pVertexBuffer.Reset();
	}
	for (PARTICLEBASE& pb : particleList)
	{
		pb.m_pTexture.Reset();
		pb.m_pVertexBuffer.Reset();
	}
}

EffectBase::BILLBOARDBASE* EffectManager::BillboardList(std::string str)
{
	if (str == "")
	{
		return &billboardList.front();	 // 先頭要素
	}
	for (BILLBOARDBASE& bb : billboardList)
	{
		if (str == bb.m_name) return &bb;
	}
	MessageBox(nullptr, "EffectManager::BillboardList()", _T("■□■ 指定のエフェクト名のエフェクトはビルボードリストにありません ■□■"), MB_OK);
	return nullptr;
}

EffectBase::PARTICLEBASE* EffectManager::ParticleList(std::string str)
{
	if (str == "")
	{
		return &particleList.front();	 // 先頭要素
	}
	for (PARTICLEBASE& pb : particleList)
	{
		if (str == pb.m_name) return &pb;
	}
	MessageBox(nullptr, "EffectManager::ParticleList()", _T("■□■ 指定のエフェクト名のエフェクトはパーティクルリストにありません ■□■"), MB_OK);
	return nullptr;
}

//=============================================================================
// 命中位置に normal 方向中心の半球状ECSパーティクルをバースト生成する。
//   旧 OOP EffectParticle::Start の拡散ロジックを踏襲。ParticleSystemが一括更新する。
//=============================================================================
void EffectManager::EmitParticleBurst(VECTOR3 pos, VECTOR3 normal)
{
	if (particleList.empty()) return;
	PARTICLEBASE& base = particleList.front();

	int count = base.m_nNum;
	if (count <= 0)  count = 20;
	if (count > 100) count = 100;

	const float size = base.m_fDestSize;
	const float life = (base.m_FrameEnd > 0) ? (base.m_FrameEnd / 60.0f) : 1.0f; // フレーム→秒

	// normal方向を中心とした半球拡散（EffectParticle::Start 準拠）
	VECTOR3 vDist;
	vDist.x = (1.0f - fabsf(normal.x)) / 2;
	vDist.y = (1.0f - fabsf(normal.y)) / 2;
	vDist.z = (1.0f - fabsf(normal.z)) / 2;

	VECTOR3 vMin, vMax;
	vMin.x = (normal.x < 0) ? (normal.x - vDist.x) : (0.0f - vDist.x);
	vMax.x = (normal.x < 0) ? (0.0f + vDist.x)     : (normal.x + vDist.x);
	vMin.y = (normal.y < 0) ? (normal.y - vDist.y) : (0.0f - vDist.y);
	vMax.y = (normal.y < 0) ? (0.0f + vDist.y)     : (normal.y + vDist.y);
	vMin.z = (normal.z < 0) ? (normal.z - vDist.z) : (0.0f - vDist.z);
	vMax.z = (normal.z < 0) ? (0.0f + vDist.z)     : (normal.z + vDist.z);

	for (int i = 0; i < count; ++i)
	{
		VECTOR3 dir;
		dir.x = ((float)rand() / RAND_MAX) * (vMax.x - vMin.x) + vMin.x;
		dir.y = ((float)rand() / RAND_MAX) * (vMax.y - vMin.y) + vMin.y;
		dir.z = ((float)rand() / RAND_MAX) * (vMax.z - vMin.z) + vMin.z;
		dir = normalize(dir);

		float   speed = (5.0f + (float)rand() / RAND_MAX) * base.m_fSpeed;
		VECTOR3 vel   = dir * (speed * 60.0f); // ParticleSystemは position += vel*dt のため秒速化

		SpawnParticle(pos, vel, life, size, 0);
	}
}

//=============================================================================
// ECSパーティクルプールを一括描画する（ParticleのRenderSystem相当）。
//   現行 EffectParticle::renderParticle と同じ Effect3D ポイントスプライト経路。
//   各 ParticleComponent を1点スプライトとしてビルボード描画する（SoAバッチ）。
//=============================================================================
void EffectManager::Draw()
{
	World& world = World::GetInstance();
	auto&  parts = world.Particles().All();
	if (parts.empty() || particleList.empty()) return;

	CDirect3D* d3d = GameDevice()->m_pD3D;
	ID3D11DeviceContext* ctx = d3d->m_pDeviceContext.Get();
	CShader* shader = GameDevice()->m_pShader;

	// ポイントスプライト用シェーダ
	ctx->VSSetShader(shader->m_pEffect3D_VS_POINT.Get(), NULL, 0);
	ctx->GSSetShader(shader->m_pEffect3D_GS_POINT.Get(), NULL, 0);
	ctx->PSSetShader(shader->m_pEffect3D_PS.Get(), NULL, 0);

	// 頂点バッファ（粒子1点）・入力レイアウト・トポロジ・サンプラ
	PARTICLEBASE& base = particleList.front();
	UINT stride = sizeof(PARTICLE_VERTEX);
	UINT offset = 0;
	ctx->IASetVertexBuffers(0, 1, base.m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	ctx->IASetInputLayout(shader->m_pEffect3D_VertexLayout.Get());
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	ctx->PSSetSamplers(0, 1, d3d->m_pSampleLinear.GetAddressOf());

	// 加算合成
	UINT mask = 0xffffffff;
	ctx->OMSetBlendState(d3d->m_pBlendStateAdd.Get(), NULL, mask);

	const MATRIX4X4 mView = GameDevice()->m_mView;
	const MATRIX4X4 mProj = GameDevice()->m_mProj;
	const VECTOR3   eye   = GameDevice()->m_vEyePt;

	for (const ParticleComponent& p : parts)
	{
		if (!p.alive) continue;

		MATRIX4X4 mWorld = GetLookatMatrix(p.position, eye);

		D3D11_MAPPED_SUBRESOURCE pData;
		CONSTANT_BUFFER_EFFECT cb;
		ZeroMemory(&cb, sizeof(cb));
		if (SUCCEEDED(ctx->Map(shader->m_pConstantBufferEffect.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			cb.mW     = XMMatrixTranspose(mWorld);
			cb.mV     = XMMatrixTranspose(mView);
			cb.mP     = XMMatrixTranspose(mProj);
			cb.fAlpha = p.alpha;
			cb.fSize  = p.size;
			memcpy_s(pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb));
			ctx->Unmap(shader->m_pConstantBufferEffect.Get(), 0);
		}
		ctx->VSSetConstantBuffers(0, 1, shader->m_pConstantBufferEffect.GetAddressOf());
		ctx->GSSetConstantBuffers(0, 1, shader->m_pConstantBufferEffect.GetAddressOf());
		ctx->PSSetConstantBuffers(0, 1, shader->m_pConstantBufferEffect.GetAddressOf());
		ctx->PSSetShaderResources(0, 1, base.m_pTexture.GetAddressOf());
		ctx->Draw(1, 0);
	}

	// 後始末（通常ブレンドへ戻す・GSリセット）
	ctx->OMSetBlendState(d3d->m_pBlendStateTrapen.Get(), NULL, mask);
	ctx->GSSetShader(NULL, NULL, 0);
}



