struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};

SamplerState samp : register(s0);
Texture2D baseTex : register(t1);	// ぼかし元の画像

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// 平滑化フィルタで画像のぼかし処理を行う。
	float x = 1.0f / 128.0f;	// UVの移動量X
	float y = 1.0f / 72.0f;	// UVの移動量Y
	color += baseTex.Sample(samp, pin.uv + float2(-x, -y)) / 9.0f;
	color += baseTex.Sample(samp, pin.uv + float2(0, -y)) / 9.0f;
	color += baseTex.Sample(samp, pin.uv + float2(x, -y)) / 9.0f;
	color += baseTex.Sample(samp, pin.uv + float2(-x, 0)) / 9.0f;
	color += baseTex.Sample(samp, pin.uv + float2(0, 0)) / 9.0f;
	color += baseTex.Sample(samp, pin.uv + float2(x, 0)) / 9.0f;
	color += baseTex.Sample(samp, pin.uv + float2(-x, y)) / 9.0f;
	color += baseTex.Sample(samp, pin.uv + float2(0, y)) / 9.0f;
	color += baseTex.Sample(samp, pin.uv + float2(x, y)) / 9.0f;

	return color;
}