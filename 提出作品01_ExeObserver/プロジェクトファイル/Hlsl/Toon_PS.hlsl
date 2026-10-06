// -----------------------------------------------------------------------
//
// トゥーンシェーディング（スタティック／スキン共通）ピクセルシェーダー
//
//                                                              Toon_PS.hlsl
// -----------------------------------------------------------------------

// グローバル変数
Texture2D g_Texture : register(t0);       // ディフューズテクスチャ

SamplerState g_samLinear : register(s0);  // サンプラー

// ワールドから射影までの変換行列・他(b0)
cbuffer global : register(b0)
{
	matrix g_mW;          // ワールド行列
	matrix g_mWVP;        // ワールドから射影までの変換行列
	float4 g_LightDir;    // ライトの方向ベクトル
	float4 g_EyePos;      // 視点位置
	float4 g_Diffuse;     // ディフューズ色
	float4 g_DrawInfo;    // 各種情報(使っていない)
};

// マテリアルカラー(b3)  ディフューズテクスチャが無いときに使用する
cbuffer cbMaterial : register(b3)
{
	float4 g_MatDiffuse;    // ディフューズ色
	float4 g_MatSpecular;   // スペキュラ色
};

// トゥーンシェーディングのパラメータ(b4)
cbuffer cbToon : register(b4)
{
	float4 g_ShadowColor;   // 影部分に掛ける色
	float  g_Threshold;     // 明暗の境界となる内積値
	float  g_SmoothFactor;  // 境界のぼかし幅
	float  g_RimPower;      // リムライトの絞り（指数）
	float  g_RimStrength;   // リムライトの寄与率
};

// ピクセルシェーダーの入力構造体
struct VS_OUTPUT
{
	float4 Pos      : SV_POSITION;
	float3 Normal   : NORMAL;
	float2 Tex      : TEXCOORD0;
	float3 PosWorld : TEXCOORD1;
};

//
// ピクセルシェーダー
//
float4 PS(VS_OUTPUT In) : SV_Target
{
	// 法線とライト方向の正規化
	float3 N = normalize(In.Normal);
	float3 L = normalize(-g_LightDir.xyz);

	// ディフューズテクスチャのサイズを得る（0ならテクスチャが無い）
	uint width, height;
	g_Texture.GetDimensions(width, height);

	// 基本色を決める。テクスチャが無いときはマテリアルカラーを使う
	float4 baseColor;
	if (width == 0)
	{
		baseColor = g_MatDiffuse * g_Diffuse;
	}
	else
	{
		baseColor = g_Texture.Sample(g_samLinear, In.Tex) * g_Diffuse;
	}

	// 明暗を段階化する（トゥーンランプ）
	float NdotL = dot(N, L);
	float ramp = smoothstep(g_Threshold, g_Threshold + g_SmoothFactor, NdotL);

	// 影色と明色を補間する
	float3 shadeColor = baseColor.rgb * g_ShadowColor.rgb;
	float3 color = lerp(shadeColor, baseColor.rgb, ramp);

	// リムライト（輪郭付近を明るくして立体感を出す）
	float3 V = normalize(g_EyePos.xyz - In.PosWorld);
	float  rim = pow(saturate(1.0f - saturate(dot(N, V))), g_RimPower) * g_RimStrength;
	color += rim;

	return float4(saturate(color), baseColor.a);
}
