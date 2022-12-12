struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};

cbuffer Light : register(b0)
{
	float4 lightDir;
};

SamplerState samp : register(s0);
Texture2D modelTex : register(t0);
Texture2D rampTex : register(t1);	// ランプテクスチャ

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = modelTex.Sample(samp, pin.uv);

	float3 N = normalize(pin.normal);
	float3 L = normalize(-lightDir.xyz);

	float diffuse = dot(N, L);

	// 内積の値を元にランプテクスチャの色を引っ張り出す
	// ※内積の結果は-1～1なので0～1に変換する
	diffuse = diffuse * 0.5f + 0.5f;
	float shadow = rampTex.Sample(samp, float2(diffuse, 0.0f));

	color.rgb *= shadow;

	return color;
}