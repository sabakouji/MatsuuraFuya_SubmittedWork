// -----------------------------------------------------------------------
// 
// ２Ｄスプライトのシェーダー
// 
//                                                              2024.4.12
//                                                          Sprite2D.hlsli
// -----------------------------------------------------------------------

// グローバル
Texture2D g_Texture: register(t0);
SamplerState g_Sampler : register(s0);

// コンスタントバッファ
cbuffer global_0:register(b0)
{
	matrix g_W;              // ワールド変換行列のみ
	float  g_ViewPortWidth;  // ビューポート（スクリーン）横サイズ
	float  g_ViewPortHeight; // ビューポート（スクリーン）縦サイズ
	float2 g_UVOffset;       // テクスチャ座標　オフセット
	float4 g_Diffuse;        // ディフューズカラー                        // 2020.1.24
	float4 g_MatInfo;        // マテリアル関連情報　x:テクスチャ有り無し  // 2017.10.8
};

// 構造体
struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 UV : TEXCOORD;
};

//
// バーテックスシェーダー
//
PS_INPUT VS( float4 Pos : POSITION ,float2 UV : TEXCOORD) 
{
	PS_INPUT Out;

	Out.Pos=mul(Pos,g_W);

	Out.Pos.x=(Out.Pos.x/g_ViewPortWidth)*2-1;
	Out.Pos.y=1-(Out.Pos.y/g_ViewPortHeight)*2;

	Out.UV = UV + g_UVOffset;

	return Out;
}


//
// ピクセルシェーダー
//
// ・テクスチャの有り無しを判断する方法に以下の2つの方法がある
//   ①　g_MatInfo.xを見て判断する。1:テクスチャ有り
//   ②　テクスチャのサイズをGetDimensions()関数で調べ、サイズが０ならテクスチャ無し
//   ここでは、①の方法をとっている
//
float4 PS( PS_INPUT In ) : SV_Target
{
	//uint width, height;
	//g_Texture.GetDimensions(width, height);  // テクスチャのサイズを得る  // -- 2020.1.15

	float4 Color;
	if (g_MatInfo.x == 1)
	//if (width != 0)
	{   // テクスチャ有りのとき
		float4 texColor = g_Texture.Sample(g_Sampler, In.UV);
		Color.rgb = texColor.rgb * g_Diffuse.rgb;
		Color.a   = texColor.a - (1 - g_Diffuse.a);     // -- 2020.1.15
	}
	else { // テクスチャなしのとき
		Color = g_Diffuse;
	}
	return saturate( Color );
}