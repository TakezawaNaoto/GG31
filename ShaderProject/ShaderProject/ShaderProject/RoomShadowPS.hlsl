// ObjectVSのVS_OUTをコピーして改変
struct PS_IN
{
	// スクリーンの座標-1～1はw成分で割ると
	// 正しく計算できる x' = x / w, y' = y / w
	// SV_POSITIONのパラメータは頂点シェーダーから
	// ピクセルシェーダーに渡される途中で勝手に計算されている
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 wPos : TEXCOORD1;
	float4 sunPos : TEXCOORD2;
};

// テクスチャ
// register(t0)はモデルに貼り付けるデフォルトのテクスチャ
Texture2D sunViewTex : register(t1);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
	// 座標をwで割る
	float2 sunUV = pin.sunPos.xy / pin.sunPos.w;
	// 画面の座標(sunUV)-1～1をテクスチャの座標0～1へ変換
	// 自分で考える
	sunUV = sunUV * 0.5f + 0.5f;
	// 画面のY座標は下から上に向かって増えるが、
	// テクスチャのY座標は上から下に向かって増えるので反転
	sunUV.y = 1.0f - sunUV.y;

	float4 sunColor = sunViewTex.Sample(samp, sunUV);
	sunColor.rgb = 1.0f - sunColor.a;
	sunColor.a = 1.0f;
	return sunColor;
}