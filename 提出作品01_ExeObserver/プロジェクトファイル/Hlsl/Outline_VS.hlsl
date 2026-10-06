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

// 輪郭線のパラメータ(b3)
cbuffer cbOutline : register(b3)
{
	float4 g_OutlineColor;    // 輪郭線の色
	float  g_OutlineWidth;    // 輪郭線の太さ（NDC単位。縦方向基準）
	float  g_OutlineAspect;   // 画面のアスペクト比（幅 / 高さ）
	float2 g_OutlinePad;      // パディング
};

// バーテックスシェーダーの入力パラメータ
struct VS_IN
{
	float3 Pos    : POSITION;   // 頂点座標
	float3 Normal : NORMAL;     // 法線
};

// バーテックスシェーダーの出力構造体
struct VS_OUTPUT
{
	float4 Pos   : SV_POSITION;   // 射影座標
	float4 Color : COLOR0;        // 輪郭線の色
};

//
// クリップ空間で法線方向へ押し出すサブ関数
//
//   clipPos    射影変換済みの頂点座標
//   clipNormal 射影変換済みの法線（正規化不要）
//
float4 ExpandOutline(float4 clipPos, float3 clipNormal)
{
	float4 outPos = clipPos;
	
	float2 nxy = clipNormal.xy;
	float  len = length(nxy);
	
	if (len >= 1.0e-6f)
	{
		float2 dir = nxy / len;
		
		dir.x /= g_OutlineAspect;
		
		outPos.xy += dir * g_OutlineWidth * abs(clipPos.w);
	}

	return outPos;
}

//
// バーテックスシェーダー
//
VS_OUTPUT VS(VS_IN In)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
	
	float4 clipPos    = mul(float4(In.Pos, 1.0f), g_mWVP);
	float3 clipNormal = mul(In.Normal, (float3x3)g_mWVP);
	
	Out.Pos = ExpandOutline(clipPos, clipNormal);
	
	Out.Color = g_OutlineColor;

	return Out;
}
