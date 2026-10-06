// -----------------------------------------------------------------------
//
// トゥーンシェーディング（スキンメッシュ用）バーテックスシェーダー
//
//                                                          ToonSkin_VS.hlsl
// -----------------------------------------------------------------------

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

// バーテックスシェーダーの入力パラメータ
struct VS_IN
{
	float3 Pos     : POSITION;      // 頂点座標
	float3 Normal  : NORMAL;        // 法線
	float2 Tex     : TEXCOORD;      // テクセル
	uint4  Bones   : BONE_INDEX;    // ボーンのインデックス
	float4 Weights : BONE_WEIGHT;   // ウェイト
};

// バーテックスシェーダーの出力構造体
struct VS_OUTPUT
{
	float4 Pos      : SV_POSITION;  // 射影座標
	float3 Normal   : NORMAL;       // ワールド空間の法線
	float2 Tex      : TEXCOORD0;    // テクセル
	float3 PosWorld : TEXCOORD1;    // ワールド空間の頂点座標
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
// バーテックスシェーダー
//
VS_OUTPUT VS(VS_IN In)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;

	// スキニング
	Skin vSkinned = SkinVert(In);

	// 頂点をワールド・ビュー・プロジェクション変換する
	Out.Pos = mul(vSkinned.Pos4, g_mWVP);

	// 法線をワールド変換する（回転成分のみ）
	Out.Normal = normalize(mul(vSkinned.Normal, (float3x3)g_mW));

	// ワールド空間の頂点座標（リムライトの視線ベクトル算出に使用）
	Out.PosWorld = mul(vSkinned.Pos4, g_mW).xyz;

	// テクスチャ座標はそのまま出力
	Out.Tex = In.Tex;

	return Out;
}
