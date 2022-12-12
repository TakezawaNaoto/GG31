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
	float2 dummy;
}

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	// 各種ベクトル
	float3 L = normalize(-lightDir.xyz);
	float3 N = normalize(pin.normal);
	float3 V = normalize(camPos.xyz - pin.wPos.xyz);

	// フォグの計算
	float dist = length(camPos.xyz - pin.wPos.xyz);
	float fog = (dist - fogStart) / fogRange;
	fog = saturate(fog);	// 0より小さい数字は0, 1より大きい数字は1にする
	float3 fogColor = float3(0.8f, 0.9f, 1.0f);

	// 拡散光
	float lambert = dot(L, N);

	color.rgb = lerp(lambert, fogColor, fog);

	return color;
}