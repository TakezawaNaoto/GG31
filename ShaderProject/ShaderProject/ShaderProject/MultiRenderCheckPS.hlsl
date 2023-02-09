struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

SamplerState samp : register(s0);
Texture2D colorTex : register(t1);
Texture2D normalTex : register(t2);
Texture2D WorldTex : register(t3);

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float2 uv = pin.uv;
	uv.y = 1.0f - uv.y;
	uv *= 2.0f;
	if (uv.y < 1.0f)
	{
		if (uv.x < 1.0f)
			color = colorTex.Sample(samp, uv);
		else
			color = colorTex.Sample(samp, uv - float2(1.0f, 0.0f));
	}
	else if (uv.x < 1.0f)
		{
			color = WorldTex.Sample(samp, uv - float2(0.0f, 1.0f));
		}

	return color;
}