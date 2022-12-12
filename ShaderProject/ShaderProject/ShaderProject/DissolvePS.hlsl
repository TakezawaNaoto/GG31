struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 wPos : TEXCOORD1;
};

cbuffer Light : register(b0)
{
	float4 lightDir;
};
cbuffer Camera : register(b1)
{
	float4 camPos;
	float fogStart;
	float fogRange;
	float disRate;
	float2 dummy;
}

SamplerState samp : register(s0);
Texture2D tex : register(t0);
Texture2D dissolveTex : register(t1);

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	// テクスチャの貼り付け
	color.rgb = tex.Sample(samp, pin.uv);

	float dissolve = dissolveTex.Sample(samp, pin.uv).r;
	// その色がrateと比べて暗いか明るいか
	float show = step(disRate, dissolve);

	color.rgb *= show;

	return color;
}