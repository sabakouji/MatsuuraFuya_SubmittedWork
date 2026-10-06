// -----------------------------------------------------------------------
//
// トゥーンシェーディング（スタティックメッシュ用）バーテックスシェーダー
//
//                                                              Toon_VS.hlsl
// -----------------------------------------------------------------------

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

// バーテックスシェーダーの入力パラメータ
struct VS_IN
{
	float3 Pos    : POSITION;   // 頂点座標
	float3 Normal : NORMAL;     // 法線
	float2 Tex    : TEXCOORD;   // テクセル
};

// バーテックスシェーダーの出力構造体
struct VS_OUTPUT
{
	float4 Pos      : SV_POSITION;  // 射影座標
	float3 Normal   : NORMAL;       // ワールド空間の法線
	float2 Tex      : TEXCOORD0;    // テクセル
	float3 PosWorld : TEXCOORD1;    // ワールド空間の頂点座標
};

//
// バーテックスシェーダー
//
VS_OUTPUT VS(VS_IN In)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;

	float4 pos4 = float4(In.Pos, 1.0f);

	// 頂点をワールド・ビュー・プロジェクション変換する
	Out.Pos = mul(pos4, g_mWVP);

	// 法線をワールド変換する（回転成分のみ）
	Out.Normal = normalize(mul(In.Normal, (float3x3)g_mW));

	// ワールド空間の頂点座標（リムライトの視線ベクトル算出に使用）
	Out.PosWorld = mul(pos4, g_mW).xyz;

	// テクスチャ座標はそのまま出力
	Out.Tex = In.Tex;

	return Out;
}
