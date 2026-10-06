#define MAX_BONE 255

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

// ボーン行列(b1)
cbuffer cbBones : register(b1)
{
	matrix BoneFramePose[MAX_BONE];   // 指定フレームでの全ての骨のポーズ行列
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
	float3 Pos     : POSITION;      // 頂点座標
	float3 Normal  : NORMAL;        // 法線
	uint4  Bones   : BONE_INDEX;    // ボーンのインデックス
	float4 Weights : BONE_WEIGHT;   // ウェイト
};

// バーテックスシェーダーの出力構造体
struct VS_OUTPUT
{
	float4 Pos   : SV_POSITION;   // 射影座標
	float4 Color : COLOR0;        // 輪郭線の色
};

// スキニング後の頂点・法線
struct Skin
{
	float4 Pos4;
	float3 Normal;
};

//
// 頂点をスキニング（ボーンによる変形）するサブ関数
//
Skin SkinVert(VS_IN In)
{
	Skin Out;
	Out.Pos4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
	Out.Normal = float3(0.0f, 0.0f, 0.0f);

	float4 pos4 = float4(In.Pos, 1.0f);

	// 4本のボーンのウェイト付き合成
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		matrix m = BoneFramePose[In.Bones[i]];
		float  w = In.Weights[i];
		Out.Pos4   += w * mul(pos4, m);
		Out.Normal += w * mul(In.Normal, (float3x3)m);
	}

	return Out;
}

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
	
	Skin vSkinned = SkinVert(In);
	
	float4 clipPos    = mul(vSkinned.Pos4, g_mWVP);
	float3 clipNormal = mul(vSkinned.Normal, (float3x3)g_mWVP);
	
	Out.Pos = ExpandOutline(clipPos, clipNormal);
	
	Out.Color = g_OutlineColor;

	return Out;
}
