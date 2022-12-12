struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(PS_IN pin) : SV_TARGET
{
	// ƒ‚ƒfƒ‹‚ÌF‚ğ’²®
	float4 color = float4(1.0f, 0.0f, 1.0f, 1.0f);
	return color;
}